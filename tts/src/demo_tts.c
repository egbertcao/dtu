#include <stdio.h>
#include "global_types.h"
#include "oc_tts.h"
#include "osa.h"
#include "oc_uart.h"

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;
static struct MBTK_TTSParams TTS_p = {1, 32000, 100, 100, 1};
static char *text = "1234567890abcdefghijkl";

static void TTS_Play_callback(int event)
{   
	OC_UART_LOG_Printf("%s ... event:%d\n", __func__,event);
	return 0;
}

void TTS_worker_thread(void * argv)
{
	OC_UART_LOG_Printf("enter %s ... \n", __func__);
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

	while(bRun)
	{
		OC_TTS_Play(&TTS_p, text, strlen(text),TTS_Play_callback);
		OSATaskSleep(10000);
	}
}

void customer_app_tts_demo(void)
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
	                 4096,120,(char*)"demoTTStask",
	                 TTS_worker_thread, NULL) != 0)
	{
		return;
	}
}


