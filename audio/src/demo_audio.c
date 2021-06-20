#include "oc_hal_feature_config.h"
#ifdef OC_AUDIO_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "osa.h"
#include "oc_uart.h"
#include "oc_audio.h"
#include "diag.h"
static OSTaskRef demoAudioWorkRef;
BOOL bRun = FALSE;

static int Audio_PCM_Play_callback(int resetLength)
{   
	OC_UART_LOG_Printf("Audio_PCM_Play_callback, resetLength=%d\n",resetLength);	
	if (resetLength ==0) 
	{
		//put max 10 frame pcm data to play buffer per time
		//OC_Audio_Play_Send_Pcm2Buffer(buffer, length);

	}

	return 0;
}
int Audio_Record_PCM_Data_callback(const char *data, int length)
{
	//put record pcm data into play buffer, length=320
	OC_Audio_Play_Send_Pcm2Buffer(data, length);

	//or if send pcm data onto internet

	return 0;
}
void Audio_TestLoopback(void)
{
	//const void *pcmfile=NULL;
	oc_audio_record_callback record_callback = Audio_Record_PCM_Data_callback;
	oc_audio_play_buffer_avail_callback  playbuffer_callback = Audio_PCM_Play_callback;
	OC_Audio_Stream_PCM_T pcmConfig={OC_AUDIO_FREQ_8000HZ, OC_AUDIO_CHANNEL_MONO};

	//start record, callback data 320 bytes put into play buffer per 20ms
	OC_Audio_Record_Start(record_callback);

	//play callback data
	OC_Audio_Play_Start (NULL, 0, OC_AUDIO_FORMAT_PCM, &pcmConfig, playbuffer_callback);

}

static int Audio_MP3_Play_callback(int resetLength)
{   
	if (resetLength ==0) 
	{
		DIAG_FILTER(AUDIO, MP3_DEC, Audio_MP3_Play_callback, DIAG_INFORMATION)
			diagPrintf("MP3 play complete!!\n")	
	}

	return 0;
}
void Audio_PlayMP3_test(void)
{    
	const void *mp3file="test.mp3";
	oc_audio_play_buffer_avail_callback  mp3Play_callback = Audio_MP3_Play_callback;
	OC_Audio_Play_Start (mp3file, 0, OC_AUDIO_FORMAT_MP3, NULL, mp3Play_callback);
}
void Audio_PlayMP3_Usb_State(BOOL usbconnect)
{    
	const void *usb_connect_mp3file="usb_connect.mp3";
	const void *usb_disconnect_mp3file="usb_disconnect.mp3";
	if (usbconnect)
	{
		OC_Audio_Play_Stop();
		OC_Audio_Play_Start (usb_connect_mp3file, 0, OC_AUDIO_FORMAT_MP3, NULL, Audio_MP3_Play_callback);
	}
	else
	{
		OC_Audio_Play_Stop();
		OC_Audio_Play_Start (usb_disconnect_mp3file, 0, OC_AUDIO_FORMAT_MP3, NULL, Audio_MP3_Play_callback);
	}
}

void Audio_work_thread(void * argv)
{
	OC_UART_LOG_Printf("enter %s ... \n", __func__);

	while(bRun)
	{
		#if 0
		//record then play
		Audio_TestLoopback();
		#endif
		
		#if 0
		//play mp3 demo
		Audio_PlayMP3_test();
		#endif

		OSATaskSleep(1000);
	}
}

void customer_app_audio_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(1024*4);
	if(TaskStack == NULL)
	{
		return;
	}
	bRun = TRUE;
	if(OSATaskCreate(&demoAudioWorkRef,
	                 TaskStack,
	                 1024*4,73,(char*)"demoAudioWorkRef",
	                 Audio_work_thread, NULL) != 0)
	{
		return;
	}
}

#endif

