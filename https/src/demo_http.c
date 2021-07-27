#include <stdio.h>
#include "global_types.h"
#include "oc_https.h"
#include "osa.h"
#include "oc_uart.h"
#include <string.h>
#include "dtu_common.h"


extern dtu_config_t g_dtu_config;

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

static void https_recv_cb( int type, unsigned int data_len, char *data)
{
	OC_UART_LOG_Printf("%s type:%d,data_len:%d\n", __func__,type,data_len);
	//print_string(data);
	if(strstr(data, "type:1")){
		received_from_server(data, data_len, TRANS_HTTP);
	}
}
static void https_event_cb(  oc_https_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

void dtu_http_send(char *message_buf)
{
	int bRet = 0;
	int dataLen = 0;
	char strDataLen[32]={0};
	
	bRet = OC_GetNetStatus();
	if(bRet == 1){
		OC_Https_URCRegister(https_recv_cb,https_event_cb);
		OC_Https_SSL_Support(0);
		OC_Https_SetUrl(g_dtu_config.currenthttp.url,2210,0);
		//strcpy(postData,"{\"deviceNum\":\"HMQZGQT000000001\",\"requestTime\":\"2020-07-11\",\"sign\":\"48D85AF5F9C787FA682F7694228AAB94\"}");
		dataLen = strlen(message_buf);
		OC_UART_LOG_Printf("%s:dataLen:%d\n", __func__,dataLen);
		sprintf(strDataLen,"%d",dataLen);
		OC_Https_SetHeader("Content-Length",strDataLen);
		OC_Https_SetHeader("Content-Type","application/json");
		OC_Https_Postdata(message_buf,dataLen);
		OC_Https_Request(HTTPS_REQUEST_POST);
		OC_UART_LOG_Printf("[%s] success.\n", __func__);
	}
}


