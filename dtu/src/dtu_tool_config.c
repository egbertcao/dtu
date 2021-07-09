#include "dtu_common.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "oc_pcac_fs.h"
#include "oc_uart.h"

static char *read_buf = NULL;
static unsigned read_flag = 0;

void device_mode_write(int deviceMode)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "deviceMode", cJSON_CreateNumber(deviceMode));
    char *config_buf = cJSON_PrintUnformatted(root);
    OC_UART_LOG_Printf("%s\n", config_buf);
    int write_ret = oc_write_file(DTU_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] write success.\n", __func__);
    }
    free(root);
}

void tool_getdtumode() 
{
    memset(read_buf, 0, 512);
    int read_ret = oc_read_file(DTU_CONFIG_FILE, read_buf);
    if (read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetDtuMode));
    cJSON *item = cJSON_ParseWithLength(read_buf, read_ret);
    cJSON_AddItemToObject(root, "msg", item);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item);
    free(root);
}

void tool_getdtudetail()
{
    deviceinfo_t deviceinfo;
    device_info_get(&deviceinfo);
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetDeviceInfo));
    cJSON_AddItemToObject(item, "imei", cJSON_CreateString(deviceinfo.imei));
    cJSON_AddItemToObject(item, "imsi", cJSON_CreateString(deviceinfo.imsi));
    cJSON_AddItemToObject(item, "cgmr", cJSON_CreateString(deviceinfo.cgmr));
    cJSON_AddItemToObject(item, "csq", cJSON_CreateString(deviceinfo.csq));
    cJSON_AddItemToObject(item, "iccid", cJSON_CreateString(deviceinfo.iccid));
    cJSON_AddItemToObject(item, "longitude", cJSON_CreateString(deviceinfo.longitude));
    cJSON_AddItemToObject(item, "latitude", cJSON_CreateString(deviceinfo.latitude));
    cJSON_AddItemToObject(root, "msg", item);
    OC_UART_LOG_Printf("[%s] %s\n",__func__, cJSON_PrintUnformatted(root));
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item);
    free(root);
}

// 每条Modbus消息用一个文件存储，并创建一个节点文件存储信息
void tool_modbus_config_write(cJSON *array)
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

void tool_modbus_config_read()
{
    int msgcount = 0;
    memset(read_buf, 0, 512);
    OC_UART_LOG_Printf("[%s]\n", __func__);
    int read_ret = oc_read_file(MODBUS_INFO_FILE, read_buf);
    if(read_ret <= 0){
        return;
    }
    OC_UART_LOG_Printf("[%s] %s\n", __func__, read_buf);
    cJSON *msgroot = cJSON_ParseWithLength(read_buf, read_ret);
	if(msgroot != NULL) {
        msgcount = cJSON_GetNumberValue(cJSON_GetObjectItem(msgroot, "msgCount"));
        OC_UART_LOG_Printf("[%s] msgcount = %d\n", __func__, msgcount);
    }
    free(msgroot);
#if 1
    cJSON *item = NULL;
    cJSON *root = cJSON_CreateObject();
    if(!cJSON_IsObject(root)) {
        OC_UART_LOG_Printf("cJSON create failed.\n");
        return;
    }
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetModbusConfig));
    int i = 0;
    for(i = 0; i< msgcount; i++){  
        char filename[20] = {0};
        memset(read_buf, 0, 512);
        memset(filename, 0, sizeof(filename));
        sprintf(filename, "modbusMsg%d", i);
        OC_UART_LOG_Printf("[%s] filename = %s\n", __func__, filename);
        read_ret = oc_read_file(filename, read_buf);
        if(read_ret <= 0){
            return;
        }
        OC_UART_LOG_Printf("[%s]  %s\n", __func__, read_buf);
        item = cJSON_ParseWithLength(read_buf, read_ret);
        cJSON_AddItemToArray(array, item);
    }
    cJSON_AddItemToObject(root, "msg", array);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    if(NULL != item) {
        free(item);
    }
    free(root);
#endif
}

void tool_mqtt_config_write(cJSON *msg)
{
    if(!cJSON_IsObject(msg)){
       OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
       return;
    }
    char *config_buf = cJSON_PrintUnformatted(msg);
    int ret = oc_write_file(MQTT_CONFIG_FILE, config_buf);
    if(ret > 0) {
        OC_UART_LOG_Printf("[%s] tool_mqtt_config_write success.\n", __func__);
    }
}

void tool_mqtt_config_read()
{
    memset(read_buf, 0, 512);
    int read_ret = oc_read_file(MQTT_CONFIG_FILE, read_buf);
    if(read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetMqttConfig));
    cJSON *item = cJSON_ParseWithLength(read_buf, read_ret);
    cJSON_AddItemToObject(root, "msg", item);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(root);
}
       
void tool_pass_config_write(cJSON *msg)
{
    if(!cJSON_IsObject(msg)){
        OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
        return;
    }
    char *config_buf = cJSON_PrintUnformatted(msg);
    OC_UART_LOG_Printf("%s\n", config_buf);
    int write_ret = oc_write_file(PASS_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] tool_pass_config_write success.\n", __func__);
    }
}

void tool_pass_config_read()
{
    memset(read_buf, 0, 512);
    int read_ret = oc_read_file(PASS_CONFIG_FILE, read_buf);
    if(read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetPassProtocol));
    cJSON *item = cJSON_ParseWithLength(read_buf, read_ret);
    cJSON_AddItemToObject(root, "msg", item);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item); 
    free(root);
}

void tool_serial_config_write(cJSON *msg)
{
    if(!cJSON_IsObject(msg)){
        OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
        return;
    }
    char *config_buf = cJSON_PrintUnformatted(msg);
    OC_UART_LOG_Printf("%s\n", config_buf);
    int write_ret = oc_write_file(SERIAL_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] tool_pass_config_write success.\n", __func__);
    }
}
void tool_serial_config_read()
{
    memset(read_buf, 0, 512);
    int read_ret = oc_read_file(SERIAL_CONFIG_FILE, read_buf);
    if(read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetSerialSetting));
    cJSON *item = cJSON_ParseWithLength(read_buf, read_ret);
    cJSON_AddItemToObject(root, "msg", item);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item);
    free(root);
}


void tool_tcp_config_write(cJSON *msg)
{
    if(!cJSON_IsObject(msg)){
        OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
        return;
    }
    
    int tcpudp = cJSON_GetNumberValue(cJSON_GetObjectItem(msg, "tcpudp"));
    char *config_buf = cJSON_PrintUnformatted(msg);
    OC_UART_LOG_Printf("%s\n", config_buf);
    char filename[20] = {0};
    sprintf(filename, "%s_%d", SOCKET_CONFIG_FILE, tcpudp);
    int write_ret = oc_write_file(filename, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] tool_pass_config_write success.\n", __func__);
    }
}
void tool_tcp_config_read(cJSON *msg)
{
    if(!cJSON_IsObject(msg)){
        OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
        return;
    }
    memset(read_buf, 0, 512);
    int tcpudp = cJSON_GetNumberValue(cJSON_GetObjectItem(msg, "tcpudp"));
    char filename[20] = {0};
    sprintf(filename, "%s_%d", SOCKET_CONFIG_FILE, tcpudp);
    int read_ret = oc_read_file(filename, read_buf);
    if(read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetTcpSetting));
    cJSON *item = cJSON_ParseWithLength(read_buf, read_ret);
    cJSON_AddItemToObject(root, "msg", item);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item);
    free(root);
}

void tool_ali_config_write(cJSON *msg)
{
    if(!cJSON_IsObject(msg)){
        OC_UART_LOG_Printf("[%s] data is not a json.\n", __func__);
        return;
    }
    char *config_buf = cJSON_PrintUnformatted(msg);
    OC_UART_LOG_Printf("%s\n", config_buf);
    int write_ret = oc_write_file(ALI_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("[%s] tool_pass_config_write success.\n", __func__);
    }
}
void tool_ali_config_read()
{
    memset(read_buf, 0, 512);
    int read_ret = oc_read_file(ALI_CONFIG_FILE, read_buf);
    if(read_ret <= 0){
        return;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "SerialFunction", cJSON_CreateNumber(GetAliSetting));
    cJSON *item = cJSON_ParseWithLength(read_buf, read_ret);
    cJSON_AddItemToObject(root, "msg", item);
    send_to_server(TRANS_SERIAL, cJSON_PrintUnformatted(root));
    free(item);
    free(root);
}

// 与串口工具交互配置
void device_config(char *serialdata, size_t size)
{
    cJSON *msg = NULL;
    cJSON *root = NULL;
    int read_ret = 0;
    int dtuMode = 0;
    if(read_flag == 0){
        read_buf = (char *)malloc(512);
        if(read_buf == NULL){
            OC_UART_LOG_Printf("[%s] malloc failed.\n",__func__);
            return;
        }
        OC_UART_LOG_Printf("[%s] malloc success.\n",__func__);
        read_flag = 1;
    }
    memset(read_buf, 0, 512);
    
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
        tool_getdtumode();
        break;
    case GetDeviceInfo:
        tool_getdtudetail();
        break;
    case SetModbusConfig:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_modbus_config_write(msg);
        break;
    case GetModbusConfig:
        tool_modbus_config_read();
        break;
    case SetMqttConfig:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_mqtt_config_write(msg);
        break;
    case GetMqttConfig:
        tool_mqtt_config_read();
        break;
    case SetPassProtocol:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_pass_config_write(msg);
        break;
    case GetPassProtocol:
        tool_pass_config_read();
        break;
    case SetSerialSetting:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_serial_config_write(msg);
        break;
    case GetSerialSetting:
        tool_serial_config_read();
        break;
    case SetTcpSetting:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_tcp_config_write(msg);
        break;
    case GetTcpSetting:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_tcp_config_read(msg);
        break;
    case SetAliSetting:
        msg = cJSON_GetObjectItem(root, "msg");
        tool_ali_config_write(msg);
        break;
    case GetAliSetting:
        tool_ali_config_read();
        break;
    default:
        break;
    }
    free(root);
}