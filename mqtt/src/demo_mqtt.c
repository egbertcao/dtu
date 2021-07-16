#include <stdio.h>
#include "global_types.h"
#include "oc_mqtt.h"
#include "osa.h"
#include "oc_uart.h"
#include "dtu_common.h"
#include <stdlib.h>
#include "oc_location.h"

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;
extern dtu_config_t g_dtu_config;

static void mqtt_recv_cb(  unsigned short packet_id, char *topic,  unsigned int payload_len, char *payload)
{
	payload[payload_len] = '\0';
	OC_UART_LOG_Printf("%s packet_id:%d,topic:%s,payload_len:%d,payload:%s\n", __func__,packet_id,topic,payload_len,payload);
	if(0 == memcmp(topic, g_dtu_config.currentmqtt.subscribe, strlen(topic))){
		received_from_server(payload, payload_len, TRANS_MQTT, packet_id);
	}
}

static void mqtt_event_cb(oc_mqtt_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

void mqtt_worker_thread(void * argv)
{
	#if 0
	int bRet,regState,netStatus = 0;
	while(bRun)
	{
		OC_NW_GetRegState(&regState);   //查询驻网成功
		if(regState)
		{
			break;
		}
		OSATaskSleep(200);//sleep 1s
	}
	OC_NetOpen();   // 激活网络
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_GetNetStatus();  // 查询是否激活成功
		if(bRet)
		{
			break;//Open  network Success!
		}
		OSATaskSleep(100);
		
	}
	OC_UART_LOG_Printf("[%s]:Open  network Success!\n", __func__);
	#endif
	int bRet = 0;
	Oc_Loc_Info location_info;
	if(g_dtu_config.passthrougth == TRNAS_THINGS){
		deviceinfo_t deviceinfo;
    	device_info_get(&deviceinfo);
		OC_Mqtt_Config("460012467925231", "460012467925432","");
		OC_Mqtt_Ipstart("182.61.41.198", 1883, 4);	
	}
	else if(g_dtu_config.passthrougth == TRANS_MQTT){
		OC_Mqtt_Config(g_dtu_config.currentmqtt.clientid, g_dtu_config.currentmqtt.username,"");
		OC_Mqtt_Ipstart(g_dtu_config.currentmqtt.address, g_dtu_config.currentmqtt.port, g_dtu_config.currentmqtt.version);
	}
	else {
		return;
	}
	OC_Mqtt_URCRegister(mqtt_recv_cb,mqtt_event_cb);
	OC_Mqtt_Connect(1, 240);
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_Mqtt_State();
		OC_UART_LOG_Printf("%s: mqtt state = %d \n", __func__,bRet);
		if(bRet==2)
		{
			OSATaskSleep(100);
			break;
		}
		OSATaskSleep(100);
	}
	OC_UART_LOG_Printf("%s:connect Success!\n", __func__);

	if(g_dtu_config.passthrougth == TRNAS_THINGS){
		OC_Mqtt_Subscribe("v1/devices/me/rpc/request/+", 0);
	}
	else{
		OC_Mqtt_Subscribe(g_dtu_config.currentmqtt.subscribe, 0);
	}

	while(bRun)
	{
		char buf[50] = {0};
		int bRet = OC_GetLocation(&location_info);
		sprintf(buf, "{\"longitude\":%s, \"latitude\":%s}", location_info.longitude,location_info.latitude);
		OC_UART_LOG_Printf("[%s] %s\n", __func__, buf);
		if(g_dtu_config.passthrougth == TRNAS_THINGS){
			bRet = OC_Mqtt_Publish("v1/devices/me/telemetry", 0, 0, buf);
			OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
		}
		else{
			bRet = OC_Mqtt_Publish(g_dtu_config.currentmqtt.publish, 0, 0, buf);
			OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
		}
		// 在此维护mqtt重连
		if(bRet == -1){
			if(2 != OC_Mqtt_State()){
				OC_Mqtt_Connect(1, 240);
			}

			while(bRun)
			{
				bRet = OC_Mqtt_State();
				OC_UART_LOG_Printf("%s: mqtt state = %d \n", __func__,bRet);
				if(bRet==2)
				{
					OSATaskSleep(100);
					break;
				}
				OSATaskSleep(100);
			}
		}
		
		// 循环发送
		OSATaskSleep(4000);
	}
}

void customer_app_mqtt_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	bRun = TRUE;
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"demoMqttTask",
	                 mqtt_worker_thread, NULL) != 0)
	{
		return;
	}
}
