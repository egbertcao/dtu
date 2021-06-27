#include "dtu_common.h"
#include "cJSON.h"

void device_mode_write(int deviceMode)
{
    cJSON *root = cJSON_CreateObject();
 
    cJSON_AddItemToObject(root, "deviceMode", cJSON_CreateNumber(deviceMode));

    char *config_buf = cJSON_GetStringValue(root);
    int write_ret = oc_write_file(DTU_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("device_mode_write success.\n");
    }
}

int device_mode_read()
{
    char config_buf[1024];
    int device_mode = 0;
    int read_ret = oc_read_file(DTU_CONFIG_FILE, config_buf);
    if (read_ret <= -1)
    {
        device_mode_write(CONFIG_MODE);
    }
    oc_read_file(DTU_CONFIG_FILE, config_buf);

    cJSON *root = cJSON_Parse(config_buf);
	if(root != NULL) {
        device_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "deviceMode"));    
    }
    free(root);
    return device_mode;
}

// 从串口接收到数据
void device_config(char *serialdata, size_t size)
{
    // readConfig
    if(strstr(serialdata, "readConfig") >= 0) {
        
    }

    // dtu_work_mode
    if(strstr(serialdata, "dtuMode:modbusMode") >= 0) {
        device_mode_write(MODBUS_MODE);
    }
     if(strstr(serialdata, "dtuMode:ConfigMode") >= 0) {
        device_mode_write(CONFIG_MODE);
    }

    // modbusConfig
    if(strstr(serialdata, "modbusConfig") >= 0) {
        
    }

    // serverConfig
    if(strstr(serialdata, "modbusConfig") >= 0) {
        
    }
}

void init_modbus_config()
{
    static char *modbus_buf = "{\"modbusConfig\":[ \
	{\"slave_address\":1,\"register_address\":3,\"count\":1,\"function\":\"aaa\",\"protocol\":1}, \
	{\"slave_address\":2,\"register_address\":3,\"count\":1,\"function\":\"bbb\",\"protocol\":1}, \
	{\"slave_address\":3,\"register_address\":3,\"count\":1,\"function\":\"ccc\",\"protocol\":1}, \
	{\"slave_address\":4,\"register_address\":3,\"count\":1,\"function\":\"ddd\",\"protocol\":1}, \
	{\"slave_address\":5,\"register_address\":3,\"count\":1,\"function\":\"eee\",\"protocol\":1}]}";

    oc_write_file(MODBUS_JSON_FILE, modbus_buf);
}