#include <stdio.h>
#include "global_types.h"
#include "oc_alimmq.h"
#include "osa.h"
#include "oc_uart.h"

#define PRINT_BUF_SIZE 65
static char print_buf[PRINT_BUF_SIZE];
static void print_string(const char * string)
{
	int len = strlen(string);
	int printed = 0;

	while (printed != len) {
		if ((len - printed) > (PRINT_BUF_SIZE - 1)) {
			memcpy(print_buf, string + printed, (PRINT_BUF_SIZE - 1));
			printed += (PRINT_BUF_SIZE - 1);
			print_buf[PRINT_BUF_SIZE - 1] = '\0';
		} else {
			sprintf(print_buf, "%s", string + printed);
			printed = len;
		}
		OC_UART_LOG_Printf("%s", print_buf);
	}
	OC_UART_LOG_Printf("\n");
}

void alimq_recv_cb(   char *topicName,int payloadlen,char* payload)
{
	OC_UART_LOG_Printf("%s topicName:%s,payloadlen:%d\n", __func__,topicName,payloadlen);
	print_string(payload);
}
void alimq_event_cb(  oc_alimq_event_type_t event_type)
{
	fatal_printf("%s event_type:%d\n", __func__,event_type);
}

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;


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
	OC_AliMq_URCRegister(alimq_recv_cb,alimq_event_cb);
	OC_AliMq_config("post-cn-4590pjm0z09.mqtt.aliyuncs.com","GID_pay_audio","HMQZGQT000000002","LTAIBlAayZeNPMf8","wyAE5ITE1fB0YRNq3ykQV16Jy9g=");
	OC_AliMq_connect();
	while(bRun)
	{
		OSATaskSleep(2000);
	}
}

void customer_app_alimmq_demo(void)
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
	                 4096,80,(char*)"demoNetOpenTask",
	                 netopen_worker_thread, NULL) != 0)
	{
		return;
	}
}


