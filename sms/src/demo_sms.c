#include <stdio.h>
#include "global_types.h"
#include "oc_sms.h"
#include "oc_sys.h"
#include "osa.h"

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

static void sms_urc_cb(char *urc)
{
	OC_UART_LOG_Printf("%s\n",urc);
}

void customer_app_sms_demo(void)
{
	int bRet= 0;
	char strCenterAddr[20]={0};
	char readMsgData[500]={0};
	OcMsgStorageInfo storageInfo;
	OSATaskSleep(6000);//wait sms ready
	OC_SMS_URCRegister(sms_urc_cb);
	bRet = OC_SMS_GetSrvCenterAddr(strCenterAddr);
	OC_UART_LOG_Printf("OC_SMS_GetSrvCenterAddr return:%d,strCenterAddr:%s\n",bRet,strCenterAddr);
	bRet = OC_SMS_SendMsg("",34,"0891683108200305F011000D91683155217061F60008B00A5DE54F5C61095FEBFF01");
	OC_UART_LOG_Printf("OC_SMS_SendMsg return:%d\n",bRet);
	bRet = OC_SMS_GetPrefStorage(&storageInfo);
	OC_UART_LOG_Printf("OC_SMS_GetPrefStorage return:%d\n",bRet);
	OC_UART_LOG_Printf("OC_SMS_GetPrefStorage %d,%d,%d\n",storageInfo.storage,storageInfo.used,storageInfo.total);
	bRet = OC_SMS_ReadMsg(1,readMsgData);
	print_string(readMsgData);
	bRet = OC_SMS_SetSmsFormatMode(1);
	OC_UART_LOG_Printf("OC_SMS_SetSmsFormatMode return:%d\n",bRet);
	bRet = OC_SMS_SendMsg("13551207166",5,"abcde");
	OC_UART_LOG_Printf("OC_SMS_SendMsg return:%d\n",bRet);
	bRet = OC_SMS_ReadMsg(1,readMsgData);
	print_string(readMsgData);
	bRet = OC_SMS_SetPrefStorage(1);
	OC_UART_LOG_Printf("OC_SMS_SetPrefStorage return:%d\n",bRet);
	bRet = OC_SMS_GetPrefStorage(&storageInfo);
	OC_UART_LOG_Printf("OC_SMS_GetPrefStorage return:%d\n",bRet);
	OC_UART_LOG_Printf("OC_SMS_GetPrefStorage %d,%d,%d\n",storageInfo.storage,storageInfo.used,storageInfo.total);
	bRet = OC_SMS_ReadMsg(1,readMsgData);
	print_string(readMsgData);
	bRet = OC_SMS_MSG_DeleteAllMsg();
	OC_UART_LOG_Printf("OC_SMS_MSG_DeleteAllMsg return:%d\n",bRet);
	bRet = OC_SMS_GetPrefStorage(&storageInfo);
	OC_UART_LOG_Printf("OC_SMS_GetPrefStorage return:%d\n",bRet);
	OC_UART_LOG_Printf("OC_SMS_GetPrefStorage %d,%d,%d\n",storageInfo.storage,storageInfo.used,storageInfo.total);
}


