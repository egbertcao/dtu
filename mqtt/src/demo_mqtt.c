#include <stdio.h>
#include "global_types.h"
#include "oc_mqtt.h"
#include "osa.h"
#include "oc_uart.h"
#include "dtu_common.h"
#include <stdlib.h>

static mqttconfig_t currentmqtt;
static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;

static void mqtt_recv_cb(  unsigned short packet_id, char *topic,  unsigned int payload_len, char *payload)
{
	payload[payload_len] = '\0';
	OC_UART_LOG_Printf("%s packet_id:%d,topic:%s,payload_len:%d,payload:%s\n", __func__,packet_id,topic,payload_len,payload);
	if(0 == memcmp(topic, currentmqtt.subscribe, strlen(topic))){
		received_from_server(payload, payload_len, TRANS_MQTT, packet_id);
	}
}

static void mqtt_event_cb(  oc_mqtt_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

void mqtt_worker_thread(void * argv)
{
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
	if(get_mqtt_param(&currentmqtt) < 0){
		OC_UART_LOG_Printf("[%s] Get mqtt param failed.\n", __func__);
		return;
	}
	OC_UART_LOG_Printf("[%s]:Open  network Success!\n", __func__);
	OC_Mqtt_URCRegister(mqtt_recv_cb,mqtt_event_cb);
	OC_UART_LOG_Printf("[%s]: clientid = %s\n", __func__, currentmqtt.clientid);
	OC_UART_LOG_Printf("[%s]: username = %s\n", __func__, currentmqtt.username);
	OC_UART_LOG_Printf("[%s]: password = %s\n", __func__, currentmqtt.password);
	OC_UART_LOG_Printf("[%s]: address = %s\n", __func__, currentmqtt.address);
	OC_UART_LOG_Printf("[%s]: port = %d\n", __func__, currentmqtt.port);
	OC_UART_LOG_Printf("[%s]: version = %d\n", __func__, currentmqtt.version);

	OC_Mqtt_Config("test123","868070040313326","");
	//OC_Mqtt_Ipstart(currentmqtt.address, currentmqtt.port, currentmqtt.version);
	OC_Mqtt_Ipstart("182.61.41.198", 1883, 4);
	OC_Mqtt_Connect(1, 240);
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_Mqtt_State();
		OC_UART_LOG_Printf("%s: mqtt state = %d \n", __func__,bRet);
		if(bRet==2)
		{
			break;
		}
		OSATaskSleep(100);
	}
	OC_UART_LOG_Printf("%s:connect Success!\n", __func__);
	//OC_Mqtt_Subscribe(currentmqtt.subscribe, 0);

	while(bRun)
	{
		// 循环发送
		int ret = OC_Mqtt_Publish("v1/devices/me/telemetry", 0, 0, "{\"device\":123}");
		OC_UART_LOG_Printf("[%s] send result = %d.\n", __func__, ret);
		OSATaskSleep(1000);
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


