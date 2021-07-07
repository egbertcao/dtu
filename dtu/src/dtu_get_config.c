#include "dtu_common.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "oc_pcac_fs.h"
#include "oc_uart.h"

int device_mode_read()
{
    char config_buf[20] = {0};
    int device_mode = 0;
    int read_ret = oc_read_file(DTU_CONFIG_FILE, config_buf);
    if(read_ret < 0) {
        return -1;
    }
    cJSON *root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        device_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "deviceMode"));    
    }
    free(root);
    return device_mode;
}

void mqtt_config_read(mqttconfig_t mqttConfig)
{
    char config_buf[1024];
    int device_mode = 0;
    int read_ret = oc_read_file(MQTT_CONFIG_FILE, config_buf);
    if(read_ret < 0){
		return;
	}

    cJSON *root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        char *address = cJSON_GetStringValue(cJSON_GetObjectItem(root, "address"));
        memcpy(mqttConfig.address, address, strlen(address));

        char *clientid = cJSON_GetStringValue(cJSON_GetObjectItem(root, "clientid"));
        memcpy(mqttConfig.clientid, clientid, strlen(clientid));

        char *username = cJSON_GetStringValue(cJSON_GetObjectItem(root, "username"));
        memcpy(mqttConfig.username, username, strlen(username));

        char *password = cJSON_GetStringValue(cJSON_GetObjectItem(root, "password"));
        memcpy(mqttConfig.password, password, strlen(password));

        mqttConfig.port = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "port"));
        mqttConfig.version = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "version"));      
    }
    free(root);
}

// 初次启动，创建配置文件
void device_config_init()
{
    // 判断是否是第一次启动，第一次启动创建文件，进入配置模式
    char config_buf[1024] = {0};
    int ret = oc_read_file(DTU_CONFIG_FILE, config_buf);
	if(ret <= 0){
		device_mode_write(CONFIG_MODE);
	
        deviceinfo_t deviceinfo;
        device_info_get(&deviceinfo);

        // mqtt init
        cJSON *mqttroot = cJSON_CreateObject();
        cJSON_AddItemToObject(mqttroot, "clientid", cJSON_CreateString(deviceinfo.imei));
        cJSON_AddItemToObject(mqttroot, "username", cJSON_CreateString(deviceinfo.imei));
        cJSON_AddItemToObject(mqttroot, "password", cJSON_CreateString(""));
        cJSON_AddItemToObject(mqttroot, "address", cJSON_CreateString("182.61.41.198"));
        cJSON_AddItemToObject(mqttroot, "port", cJSON_CreateNumber(1883));
        cJSON_AddItemToObject(mqttroot, "version", cJSON_CreateNumber(4));
        char *mqttbuf = cJSON_PrintUnformatted(mqttroot);
        OC_UART_LOG_Printf("%s\n", mqttbuf);
        int write_ret = oc_write_file(MQTT_CONFIG_FILE, mqttbuf);
        if(write_ret > 0) {
            OC_UART_LOG_Printf("mqtt init success.\n");
        }
        free(mqttroot);
    }
}