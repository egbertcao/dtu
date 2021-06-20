#include <stdio.h>
#include "global_types.h"
#include "oc_alimqtt.h"
#include "osa.h"
#include "oc_uart.h"

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;
static void mqtt_recv_cb(  unsigned short packet_id, char *topic,  unsigned int payload_len, char *payload)
{
	OC_UART_LOG_Printf("%s packet_id:%d,topic:%s,payload_len:%d,payload:%s\n", __func__,packet_id,topic,payload_len,payload);
	OC_Alimqtt_Puback(packet_id);
}
static void mqtt_event_cb(  oc_mqtt_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

static void netopen_worker_thread(void * argv)
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
	OC_Alimqtt_URCRegister(mqtt_recv_cb,mqtt_event_cb);
	OC_Alimqtt_Auth("a1fSTEcRSgP","LqNGI0zNNDeDfcDU","2eSgsZQTm0AYCu5l8KFL","61IWeKxPMzD606ouW4bHydrnwuC2X5K9",1);
	OC_Alimqtt_Connect();
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_Alimqtt_State();
		if(bRet==2)
		{
			break;
		}
		OSATaskSleep(100);
	}
	OC_UART_LOG_Printf("%s:connect Success!\n", __func__);
	OC_Alimqtt_Subscribe("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/get",0);
	OC_Alimqtt_Subscribe("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/update",0);
	OC_Alimqtt_Publish("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/get",0,"helloworld");
	OC_Alimqtt_Publish("/a1fSTEcRSgP/2eSgsZQTm0AYCu5l8KFL/user/update",0,"helloworldtest");
	while(bRun)
	{
		OSATaskSleep(2000);
	}
}

void customer_alimqtt_manualAck_demo(void)
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
	                 4096,80,(char*)"demoalimqttmanualAckTask",
	                 netopen_worker_thread, NULL) != 0)
	{
		return;
	}
}


