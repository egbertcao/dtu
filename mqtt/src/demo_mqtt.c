#include <stdio.h>
#include "global_types.h"
#include "oc_mqtt.h"
#include "osa.h"
#include "oc_uart.h"
#include "dtu_common.h"
#include <stdlib.h>
#include "oc_location.h"
#include "cJSON.h"
#include "oc_net.h"

static OSTaskRef MqttWorkerRef;
static OSTaskRef MqttMonitorWorkerRef;
static BOOL MqttWorkRun = FALSE;
static BOOL MqttMonitorRun = FALSE;
extern dtu_config_t g_dtu_config;

static void mqtt_recv_cb(  unsigned short packet_id, char *topic,  unsigned int payload_len, char *payload)
{
	payload[payload_len] = '\0';
	OC_UART_LOG_Printf("%s packet_id:%d,topic:%s,payload_len:%d,payload:%s\n", __func__,packet_id,topic,payload_len,payload);
	if(0 == memcmp(topic, g_dtu_config.currentmqtt.subscribe, strlen(topic))){
		received_from_server(payload, payload_len, TRANS_MQTT);
	}
}

static void mqtt_event_cb(oc_mqtt_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

// 只负责连接和订阅
static int dtu_mqtt_connect()
{
	int bRet = 0;
	OC_Mqtt_URCRegister(mqtt_recv_cb,mqtt_event_cb);
	if(g_dtu_config.passthrougth == TRNAS_THINGS){
		OC_UART_LOG_Printf("[%s]: imei = %s, len = %d\n", __func__,	g_dtu_config.deviceinfo.imei, strlen(g_dtu_config.deviceinfo.imei));
		char imeibuf[30] = {0};
		sprintf(imeibuf, "%s", g_dtu_config.deviceinfo.imei);
		OC_Mqtt_Config(imeibuf, imeibuf,"");
		OC_Mqtt_Ipstart("182.61.41.198", 1883, 4);
	}
	else if(g_dtu_config.passthrougth == TRANS_MQTT){
		OC_Mqtt_Config(g_dtu_config.currentmqtt.clientid, g_dtu_config.currentmqtt.username, g_dtu_config.currentmqtt.password);
		OC_Mqtt_Ipstart(g_dtu_config.currentmqtt.address, g_dtu_config.currentmqtt.port, g_dtu_config.currentmqtt.version);
	}
	else {
		return -1;
	}
	OSATaskSleep(100);
	int connectFlag = 0;
	OC_Mqtt_Connect(1, 240);
	OC_UART_LOG_Printf("[%s] OC_Mqtt_Connect \n", __func__);
	while(MqttWorkRun)
	{
		bRet = OC_Mqtt_State();
		OC_UART_LOG_Printf("[%s] mqtt state = %d \n", __func__,bRet);
		if(bRet==0) {
			connectFlag++;
			if(connectFlag >= 20) {
				OC_UART_LOG_Printf("[%s] OC_Mqtt_Ipclose \n", __func__);		
				OC_Mqtt_Ipclose();  // 连接失败，释放mqtt资源
				return -1;
			}
		}
		if(bRet==2)
		{
			OSATaskSleep(100);
			break;
		}
		OSATaskSleep(100);
	}
	OC_UART_LOG_Printf("[%s] connect Success!\n", __func__);

	if(g_dtu_config.passthrougth == TRNAS_THINGS){
		OC_Mqtt_Subscribe("v1/devices/me/rpc/request/+", 0);
	}
	else{
		OC_Mqtt_Subscribe(g_dtu_config.currentmqtt.subscribe, 0);
	}
	return 0;
}


// mqtt发送线程，判断mqtt状态正常则发送，只有发送功能
void mqtt_worker_thread(void * argv)
{
	while(MqttWorkRun)
	{
		int bRet = OC_Mqtt_State();
		OC_UART_LOG_Printf("[%s] mqttstatus = %d\n", __func__, bRet);
		if(bRet == 2){	
			char buf[200] = {0};
			#if 0
			Oc_Loc_Info location_info;
			OC_GetLocation(&location_info);
			sprintf(buf, "{\"longitude\":%s, \"latitude\":%s}", location_info.longitude,location_info.latitude);
			cJSON *root = cJSON_Parse(buf);
			if(!cJSON_IsObject(root)){
				OC_UART_LOG_Printf("[%s] serialdata is not json.\n",__func__);
				continue;
			}
			#endif
			sprintf(buf, "{\"longitude\":%s, \"latitude\":%s}", "10", "10");
			OC_UART_LOG_Printf("[%s] %s\n", __func__, buf);
			if(g_dtu_config.passthrougth == TRNAS_THINGS){
				bRet = OC_Mqtt_Publish("v1/devices/me/telemetry", 0, 0, buf);
				OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
			}
			else{
				bRet = OC_Mqtt_Publish(g_dtu_config.currentmqtt.publish, 0, 0, buf);
				OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
			}
		}
		// 循环发送
		OSATaskSleep(4000); // 每20s发送一次地理位置
	}
	
}

// mqtt监控线程，判断mqtt状态不正常则发起重连
void mqtt_monitor_thread(void * argv)
{
	OC_UART_LOG_Printf("[%s]: Start Mqtt Thread Monitor.\n", __func__);
	while (MqttMonitorRun){
		int netstatus = OC_GetNetStatus();
		if(netstatus == 1) { // 网络恢复后再重新连接
			int bRet = OC_Mqtt_State();
			if(bRet == 3 || bRet == 0 ){  // 只有在mqtt断开连接时才发起重新连接
				// 工作过程中发生中断，释放mqtt资源
				OC_Mqtt_Disconnect();	
				OC_Mqtt_Ipclose(); 
				OC_UART_LOG_Printf("[%s]: OC_Mqtt_Ipclose.\n", __func__);
				while (1)
				{
					int ret = dtu_mqtt_connect();
					if(ret == 0){
						break;
					}
					OSATaskSleep(2000);
				}
			}
		}
		OSATaskSleep(2000); //每10s查询一次MQTT状态
	}
}

void customer_app_mqtt_start(void)
{
	OC_UART_LOG_Printf("[%s] mqtt Start!\n", __func__);
	void *MqttMonitorTaskStack;
	MqttMonitorTaskStack=malloc(4096);
	if(MqttMonitorTaskStack == NULL)
	{
		return;
	}
	MqttMonitorRun = TRUE;
	if(OSATaskCreate(&MqttMonitorWorkerRef,
	                 MqttMonitorTaskStack,
	                 4096,80,(char*)"MqttMonitorTask",
	                 mqtt_monitor_thread, NULL) != 0)
	{
		return;
	}

	void *MqttWorkTaskStack;
	MqttWorkTaskStack=malloc(4096);
	if(MqttWorkTaskStack == NULL)
	{
		return;
	}
	MqttWorkRun = TRUE;
	if(OSATaskCreate(&MqttWorkerRef,
					MqttWorkTaskStack,
					4096,80,(char*)"MqttWorkTask",
					mqtt_worker_thread, NULL) != 0)
	{
		return;
	}
}
