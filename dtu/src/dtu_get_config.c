#include "dtu_common.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "oc_pcac_fs.h"
#include "oc_uart.h"
#include "global_types.h"
#include "osa.h"
#include "modbus.h"

void arr_rpttn(int *pArr, int n)
{
	int i, j, k;
	int cnt = 0;
	for(i=0; i<n-cnt; i++)
	{
		for(j=i+1; j<n-cnt; j++)
		{
			if(pArr[i] == pArr[j])
			{
				for(k=j; k<n-cnt-1; k++)
				{
					pArr[k] = pArr[k+1];
				}
				j--;
				cnt++;
				pArr[k] = 0;
			}
		}
	}
	
	return;
}
						  
int get_modbus_slaves(void *slaves, unsigned int *slave_ids, unsigned int *slave_count)
{
	cJSON *item = NULL;
	unsigned int msg_count = 0;
	unsigned int s_count = 0;
    char buf[20] = {0};
	char modbusinfo_buf[1024];
	char filename[20] = {0};
	int read_ret = oc_read_file(MODBUS_INFO_FILE, buf);
	cJSON *root = cJSON_ParseWithLength(buf, read_ret);
	if(root != NULL) {
        msg_count = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "msgCount"));
    }

	int i = 0;
	for(i = 0; i< msg_count; i++){
		memset(modbusinfo_buf, 0, sizeof(modbusinfo_buf));
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "modbusMsg%d", i);
		read_ret = oc_read_file(filename, modbusinfo_buf);
		item = cJSON_ParseWithLength(modbusinfo_buf, read_ret);
		if(item != NULL){
			slave_msg_t *slave_item = (slave_msg_t *)slaves;
			slave_item->s_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "slave_address"));
            slave_item->function_code = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "modbus_function"));
			slave_item->r_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_address"));
			slave_item->protocol = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "slave_protocol"));
			slave_item->count = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_count"));
			char *function = cJSON_GetStringValue(cJSON_GetObjectItem(item, "slave_function"));
			memcpy(slave_item->function, function, strlen(function));
            char *data_deal = cJSON_GetStringValue(cJSON_GetObjectItem(item, "data_deal"));
			memcpy(slave_item->data_deal, function, strlen(data_deal));
			
			slave_ids[i] = slave_item->s_address;
			s_count = s_count + 1;
			slaves = slaves + sizeof(slave_msg_t);
		}
	}
	
	free(item);
	free(root);
	arr_rpttn(slave_ids, s_count);
	for(i=0;i<s_count; i++)
	{
		if(slave_ids[i] != 0){
			*slave_count = *slave_count+ 1;
		}
	}
	return msg_count;
}

int get_device_mode()
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

void get_mqtt_param(mqttconfig_t mqttConfig)
{
    char config_buf[1024] = {0};
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

void get_serial_param(serialconfig_t serialConfig)
{
    char config_buf[1024] = {0};
    int read_ret = oc_read_file(SERIAL_CONFIG_FILE, config_buf);
    if(read_ret < 0){
		return;
	}

    cJSON *root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {

        serialConfig.baudrate = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_baud"));
        serialConfig.databits = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_data"));
        serialConfig.stopbits = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_stop"));
        serialConfig.parity = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_Parity"));      
    }
    free(root);
}

void get_socket_param(socketconfig_t socketConfig)
{
    char config_buf[1024] = {0};
    cJSON *root = NULL;
    int read_ret = oc_read_file("sockerconfig.json1", config_buf);
    if(read_ret < 0){
		return;
	}
    root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        socketConfig.tcpport = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "ip_port"));
        char *ipaddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "ip_address"));
        memcpy(socketConfig.tcpaddress, ipaddress, strlen(ipaddress));
    }

    memset(config_buf, 0 ,sizeof(config_buf));
    read_ret = oc_read_file("sockerconfig.json2", config_buf);
    if(read_ret < 0){
		return;
	}
    root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        socketConfig.udpport = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "ip_port"));
        char *ipaddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "ip_address"));
        memcpy(socketConfig.udpaddress, ipaddress, strlen(ipaddress));
    }
    free(root);
}

void get_ali_param(aliconfig_t aliConfig)
{
    char config_buf[1024] = {0};
    int read_ret = oc_read_file(ALI_CONFIG_FILE, config_buf);
    if(read_ret < 0){
		return;
	}

    cJSON *root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        char *product_key = cJSON_GetStringValue(cJSON_GetObjectItem(root, "product_key"));
        memcpy(aliConfig.product_key, product_key, strlen(product_key));
        char *product_secret = cJSON_GetStringValue(cJSON_GetObjectItem(root, "product_secret"));
        memcpy(aliConfig.product_secret, product_secret, strlen(product_secret));
        char *device_name = cJSON_GetStringValue(cJSON_GetObjectItem(root, "device_name"));
        memcpy(aliConfig.device_name, device_name, strlen(device_name));
        char *device_secret = cJSON_GetStringValue(cJSON_GetObjectItem(root, "device_secret"));
        memcpy(aliConfig.device_secret, device_secret, strlen(device_secret));
        aliConfig.puback_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "puback_mode"));    
    }
    free(root);
}

int get_passthrough_param()
{
    char config_buf[20] = {0};
    int protocol = 0;
    int read_ret = oc_read_file(PASS_CONFIG_FILE, config_buf);
    if(read_ret < 0){
		return -1;
	}

    cJSON *root = cJSON_ParseWithLength(config_buf, read_ret);
	if(root != NULL) {
        protocol = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "passProtocol"));    
    }
    free(root);
    return protocol;
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