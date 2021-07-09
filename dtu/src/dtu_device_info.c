#include <stdio.h>
#include "global_types.h"
#include "dtu_common.h"
#include "oc_at.h"
#include "osa.h"
#include "oc_pcac_fs.h"

static void delete_char(char str[],char target){
	int i,j;
	for(i=j=0;str[i]!='\0';i++){
		if(str[i]!=target){
			str[j++]=str[i];
		}
	}
	str[j]='\0';
}

static void device_imei_get(char *imei)
{
    int ret;
    char atcmd[64];
    char respBuf[100];
    memset(atcmd, 0, sizeof(atcmd));
    memset(respBuf, 0, sizeof(respBuf));
    sprintf(atcmd, "AT+GSN\r");
    ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
    delete_char(respBuf, '\"');
    OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
    memcpy(imei, respBuf, strlen(respBuf));
}

static void device_imsi_get(char *imsi)
{
    int ret;
    char atcmd[64];
    char respBuf[100];
    memset(atcmd, 0, sizeof(atcmd));
    memset(respBuf, 0, sizeof(respBuf));
    sprintf(atcmd, "AT+CIMI\r");
    ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
    delete_char(respBuf, '\"');
    OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
    if(strstr(respBuf, "ERROR") != NULL) {
        memcpy(imsi, "", strlen(""));
    }
    else {
        memcpy(imsi, respBuf, strlen(respBuf));
    }
}

static void device_cgmr_get(char *cgmr)
{
    int ret;
    char atcmd[64];
    char respBuf[100];
    memset(atcmd, 0, sizeof(atcmd));
    memset(respBuf, 0, sizeof(respBuf));
    sprintf(atcmd, "AT+CGMR\r");
    ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
    delete_char(respBuf, '\"');
    OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
    if(strstr(respBuf, "ERROR") != NULL) {
        memcpy(cgmr, "", strlen(""));
    }
    else {
        memcpy(cgmr, respBuf, strlen(respBuf));
    }
}

static void device_iccid_get(char *iccid)
{
    int ret;
    char atcmd[64];
    char respBuf[100];
    memset(atcmd, 0, sizeof(atcmd));
    memset(respBuf, 0, sizeof(respBuf));
    sprintf(atcmd, "AT+ICCID\r");
    ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
    delete_char(respBuf, '\"');
    OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
    if(strstr(respBuf, "ERROR") != NULL) {
        memcpy(iccid, "", strlen(""));
    }
    else {
        memcpy(iccid, respBuf, strlen(respBuf));
    }
}

static void device_csq_get(char *csq)
{
    int ret;
    char atcmd[64];
    char respBuf[100];
    memset(atcmd, 0, sizeof(atcmd));
    memset(respBuf, 0, sizeof(respBuf));
    sprintf(atcmd, "AT+CSQ\r");
    ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
    delete_char(respBuf, '\"');
    OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
    if(strstr(respBuf, "ERROR") != NULL) {
        memcpy(csq, "", strlen(""));
    }
    else {
        memcpy(csq, respBuf, strlen(respBuf));
    }
}

static void device_creg_get(char *creg)
{
    int ret;
    char atcmd[64];
    char respBuf[100];
    memset(atcmd, 0, sizeof(atcmd));
    memset(respBuf, 0, sizeof(respBuf));
    sprintf(atcmd, "AT+CREG?\r");
    ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
    delete_char(respBuf, '\"');
    OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
    if(strstr(respBuf, "ERROR") != NULL) {
        memcpy(creg, "", strlen(""));
    }
    else {
        memcpy(creg, respBuf, strlen(respBuf));
    }
}

void device_info_get(deviceinfo_t *deviceinfo)
{
    memset(deviceinfo, 0, sizeof(deviceinfo_t));
    device_imei_get(deviceinfo->imei);
    device_imsi_get(deviceinfo->imsi);
    device_cgmr_get(deviceinfo->cgmr);
    device_iccid_get(deviceinfo->iccid);
    device_creg_get(deviceinfo->creg);
    device_csq_get(deviceinfo->csq);
}