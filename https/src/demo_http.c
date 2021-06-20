#include <stdio.h>
#include "global_types.h"
#include "oc_https.h"
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

static void https_recv_cb(   int type, unsigned int data_len, char *data)
{
	OC_UART_LOG_Printf("%s type:%d,data_len:%d\n", __func__,type,data_len);
	print_string(data);
}
static void https_event_cb(  oc_https_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}


static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;


static void netopen_worker_thread(void * argv)
{
	int bRet,regState,netStatus = 0;
	int dataLen = 0;
	char strDataLen[32]={0};
	char postData[256]={0};
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
	OC_Https_URCRegister(https_recv_cb,https_event_cb);
	OC_Https_SSL_Support(0);
	OC_Https_SetUrl("http://182.148.114.87/file/xph.txt",2210,0);
	strcpy(postData,"{\"deviceNum\":\"HMQZGQT000000001\",\"requestTime\":\"2020-07-11\",\"sign\":\"48D85AF5F9C787FA682F7694228AAB94\"}");
	dataLen = strlen(postData);
	OC_UART_LOG_Printf("%s:dataLen:%d\n", __func__,dataLen);
	sprintf(strDataLen,"%d",dataLen);
	OC_Https_SetHeader("Content-Length",strDataLen);
	OC_Https_SetHeader("Content-Type","application/json");
	OC_Https_Postdata(postData,dataLen);
	OC_Https_Request(HTTPS_REQUEST_POST);
}

void customer_app_http_demo(void)
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


