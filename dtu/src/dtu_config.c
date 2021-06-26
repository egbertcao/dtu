#include "dtu_common.h"
#include "cJSON.h"

void init_config_json()
{
    cJSON *root = cJSON_CreateObject();
 
    cJSON_AddItemToObject(root, "deviceMode", cJSON_CreateNumber(0));

    char *config_buf = cJSON_GetStringValue(root);
    int write_ret = oc_write_file(DTU_CONFIG_FILE, config_buf);
    if(write_ret > 0) {
        OC_UART_LOG_Printf("init_config_json success.\n");
    }
}

void init_config(dtu_config_t *dtu_config)
{
    // 读取配置文件
    char config_buf[1024];
    int read_ret = oc_read_file(DTU_CONFIG_FILE, config_buf);
    if (read_ret <= -1)
    {
        init_config_json();
    }
    oc_read_file(DTU_CONFIG_FILE, config_buf);

    cJSON *root = cJSON_Parse(config_buf);
	if(root != NULL) {
        dtu_config->dtu_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "deviceMode"));    
    }
}

void init_modbus_config()
{
    static char *modbus_buf = "{\"msg\":[ \
	{\"slave_address\":1,\"register_address\":3,\"count\":1,\"function\":\"aaa\",\"protocol\":1}, \
	{\"slave_address\":2,\"register_address\":3,\"count\":1,\"function\":\"bbb\",\"protocol\":1}, \
	{\"slave_address\":3,\"register_address\":3,\"count\":1,\"function\":\"ccc\",\"protocol\":1}, \
	{\"slave_address\":4,\"register_address\":3,\"count\":1,\"function\":\"ddd\",\"protocol\":1}, \
	{\"slave_address\":5,\"register_address\":3,\"count\":1,\"function\":\"eee\",\"protocol\":1}]}";

    oc_write_file(MODBUS_JSON_FILE, modbus_buf);
}