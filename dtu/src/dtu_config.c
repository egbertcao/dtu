#include "dtu_common.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "oc_pcac_fs.h"
#include "oc_uart.h"

void device_mode_write(int deviceMode)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "deviceMode", cJSON_CreateNumber(deviceMode));
    char *config_buf = cJSON_PrintUnformatted(root);
    OC_UART_LOG_Printf("%s\n", config_buf);
    int write_ret = oc_write_file(DTU_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] device_mode_write success.\n", __func__);
    }
    free(root);
}

int device_mode_read()
{
    char config_buf[20] = {0};
    int device_mode = 0;
    int read_ret = oc_read_file(DTU_CONFIG_FILE, config_buf);
    if(read_ret < 0) {
        return -1;
    }
    #if 1
    cJSON *root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        device_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "deviceMode"));    
    }
    free(root);
    #endif
    return device_mode;
}

void M_getdtumode() 
{
    cJSON *root = cJSON_CreateObject();
    cJSON *item = NULL;
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetDtuMode));
    char config_buf[50] = {0};
    int read_ret = oc_read_file(DTU_CONFIG_FILE, config_buf);
    if(read_ret > 0) {
        item = cJSON_ParseWithLength(config_buf, read_ret);
        cJSON_AddItemToObject(root, "msg", item);
        send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    }
    free(item);
    free(root);
}

void M_getdtudetail()
{
    deviceinfo_t deviceinfo;
    device_info_get(&deviceinfo);
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetDeviceInfo));
    cJSON_AddItemToObject(item, "imei", cJSON_CreateString(deviceinfo.imei));
    cJSON_AddItemToObject(item, "imsi", cJSON_CreateString(deviceinfo.imsi));
    cJSON_AddItemToObject(root, "msg", item);
    OC_UART_LOG_Printf("[%s] %s\n",__func__, cJSON_PrintUnformatted(root));
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item);
    free(root);
}

// 每条Modbus消息用一个文件存储，并创建一个节点文件存储信息
void modbus_config_write(cJSON *array)
{
    cJSON *item = NULL;
    char filename[20] = {0};
    unsigned int slave_ids[20] = {0};
    unsigned int array_size = 0;
    if(cJSON_IsArray(array)){
        array_size = cJSON_GetArraySize(array);
        size_t i = 0;
        for (i = 0; i < array_size; i++)
        {
            if(i >= MAX_SLAVE){
                break;
            }
            item = cJSON_GetArrayItem(array, i);
            char *config_buf = cJSON_PrintUnformatted(item);
            sprintf(filename, "modbusMsg%d", i);
            oc_write_file(filename, config_buf);
            OC_UART_LOG_Printf("[%s] %d modbus Msg write success.\n",__func__, i);
        }
	}
    // 将信息写入节点文件
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "msgCount", cJSON_CreateNumber(array_size));
    char *sizebuf = cJSON_PrintUnformatted(root);
    OC_UART_LOG_Printf("%s\n", sizebuf);
    int write_ret = oc_write_file(MODBUS_INFO_FILE, sizebuf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] write success.\n", __func__);
    }
    free(root);
    free(item);
}

void modbus_config_read()
{
    int msgcount = 0;
    char buf[20] = {0};
    char modbusinfo_buf[1024] = {0};
    char filename[20] = {0};
    int read_ret = oc_read_file(MODBUS_INFO_FILE, buf);
    if(read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_ParseWithLength(buf, read_ret);
	if(root != NULL) {
        msgcount = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "msgCount"));
    }

    int i = 0;
    for(i = 0; i< msgcount; i++){
        memset(modbusinfo_buf, 0, sizeof(modbusinfo_buf));
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "modbusMsg%d", i);
        read_ret = oc_read_file(filename, modbusinfo_buf);
        if(read_ret > 0) {
            send_to_server(TRANS_SERIAL, modbusinfo_buf);
        }
    }
    free(root);
}

void mqtt_config_write(cJSON *mqttConfig)
{
    if(cJSON_IsObject(mqttConfig)){
        char *config_buf = cJSON_PrintUnformatted(mqttConfig);
        int ret = oc_write_file(MQTT_CONFIG_FILE, config_buf);
        if(ret > 0) {
            OC_UART_LOG_Printf("[%s] mqtt_config_write success.\n", __func__);
        }
    }
    else{
        OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
    }
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

// 与串口工具交互配置
void device_config(char *serialdata, size_t size)
{
    cJSON *msg = NULL;
    cJSON *root = NULL;
    int read_ret = 0;
    int dtuMode = 0;
    char config_buf[1024] = {0};
    
    OC_UART_LOG_Printf("[%s] %s\n",__func__, serialdata);
    root = cJSON_ParseWithLength(serialdata, size);
    if(!cJSON_IsObject(root)){
        OC_UART_LOG_Printf("[%s] serialdata is not json.\n",__func__);
        return;
    }
    int functionCode = (unsigned int)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "SerialFunction"));
    
    switch (functionCode)
    {
    case GetDtuMode:
        M_getdtumode();
        break;
    case GetDeviceInfo:
        M_getdtudetail();
        break;
    //{"serialFunction:13, "Msg":[{"slave_address":1, "register_address":3, "count":1, "function":"aaa","protocol":1},]}
    case SetModbusConfig:
        msg = cJSON_GetObjectItem(root, "msg");
        if (NULL != msg){
            modbus_config_write(msg);
        }
        break;
    case GetModbusConfig:
        modbus_config_read();
        break;
    case SetMqttConfig:
        msg = cJSON_GetObjectItem(root, "msg");
        mqtt_config_write(msg);
        break;
    case GetMqttConfig:
        memset(config_buf, 0, sizeof(config_buf));
        read_ret = oc_read_file(MQTT_CONFIG_FILE, config_buf);
        if(read_ret > 0) {
            send_to_server(TRANS_SERIAL, config_buf);
        }
        break;
    default:
        break;
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
            OC_UART_LOG_Printf("mqtt_config_write success.\n");
        }
        free(mqttroot);
    }
}