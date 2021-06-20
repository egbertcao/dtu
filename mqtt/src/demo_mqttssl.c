#include <stdio.h>
#include "global_types.h"
#include "oc_mqtt.h"
#include "osa.h"
#include "oc_uart.h"

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;
static void mqtt_recv_cb(  unsigned short packet_id, char *topic,  unsigned int payload_len, char *payload)
{
	OC_UART_LOG_Printf("%s packet_id:%d,topic:%s,payload_len:%d,payload:%s\n", __func__,packet_id,topic,payload_len,payload);
}
static void mqtt_event_cb(  oc_mqtt_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

static void mqttssl_worker_thread(void * argv)
{
	int bRet,regState,netStatus = 0;
	while(bRun)
	{
		OC_NW_GetRegState(&regState);
		if(regState)
		{
			break;
		}
		OSATaskSleep(200);//sleep 1s
	}
	OC_NetOpen();
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_GetNetStatus();
		if(bRet)
		{
			break;//Open  network Success!
		}
		OSATaskSleep(100);
		
	}
	OC_UART_LOG_Printf("%s:Open  network Success!\n", __func__);
	OC_Mqtt_URCRegister(mqtt_recv_cb,mqtt_event_cb);
	OC_Mqtt_ssl(1, 0);
	OC_Mqtt_Config("MQTT_FX_Client_SSL","admin","admin");
	OC_Mqtt_Ipstart("182.61.41.198", 1883, 4);
	OC_Mqtt_Connect(1, 60);
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
	OC_Mqtt_Subscribe("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/get",0);
	OC_Mqtt_Subscribe("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/update",0);
	while(bRun) {
	OC_Mqtt_Publish("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/get",0,0,"helloworld");
	OC_Mqtt_Publish("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/update",0,0,"helloworldtest");
	OSATaskSleep(2000);
	}
}

void customer_app_mqttssl_demo(void)
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
	                 4096,80,(char*)"demoMqttsslTask",
	                 mqttssl_worker_thread, NULL) != 0)
	{
		return;
	}
}


