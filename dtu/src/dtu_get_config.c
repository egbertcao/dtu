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

static char *read_buf = NULL;
static unsigned read_flag = 0;

static void arr_rpttn(int *pArr, int n)
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
	unsigned int msg_count = 0;
	unsigned int s_count = 0;
	char filename[20] = {0};
    memset(read_buf,0,512);
	int read_ret = oc_read_file(MODBUS_INFO_FILE, read_buf);
    if(read_ret <= 0){
        return;
    }
	cJSON *msgroot = cJSON_ParseWithLength(read_buf, read_ret);
	if(msgroot != NULL) {
        msg_count = cJSON_GetNumberValue(cJSON_GetObjectItem(msgroot, "msgCount"));
    }

	int i = 0;
    cJSON *item = NULL;
	for(i = 0; i< msg_count; i++){    
        memset(read_buf, 0, 512);
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "modbusMsg%d", i);
		read_ret = oc_read_file(filename, read_buf);
        if(read_ret <= 0) {
            return;
        }
		item = cJSON_ParseWithLength(read_buf, read_ret);
		if(item != NULL){
			slave_msg_t *slave_item = (slave_msg_t *)slaves;
			slave_item->s_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "slave_address"));
            slave_item->function_code = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "modbus_function"));
			slave_item->r_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_address"));
			slave_item->count = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_count"));
            slave_item->multiply = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "data_multiply"));
            slave_item->endian = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "endian"));
			char *function = cJSON_GetStringValue(cJSON_GetObjectItem(item, "slave_function"));
			memcpy(slave_item->function, function, strlen(function));
			
			slave_ids[i] = slave_item->s_address;
			s_count = s_count + 1;
			slaves = slaves + sizeof(slave_msg_t);
		}
	}
	
	free(item);
	free(msgroot);
	arr_rpttn(slave_ids, s_count);
	for(i=0;i<s_count; i++)
	{
		if(slave_ids[i] != 0){
			*slave_count = *slave_count+ 1;
		}
	}
	return msg_count;
}

static int get_device_mode()
{
    memset(read_buf,0,512);
    int device_mode = 0;
    int read_ret = oc_read_file(DTU_CONFIG_FILE, read_buf);
    if(read_ret < 0) {
        return -1;
    }
    cJSON *root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        device_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "deviceMode"));    
    }
    free(root);
    return device_mode;
}

static int get_mqtt_param(mqttconfig_t *mqttConfig)
{
    memset(read_buf,0,512);
    int read_ret = oc_read_file(MQTT_CONFIG_FILE, read_buf);
    if(read_ret < 0){
		return;
	}

    cJSON *root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        char *address = cJSON_GetStringValue(cJSON_GetObjectItem(root, "address"));
        memcpy(mqttConfig->address, address, strlen(address));

        char *clientid = cJSON_GetStringValue(cJSON_GetObjectItem(root, "clientid"));
        memcpy(mqttConfig->clientid, clientid, strlen(clientid));

        char *username = cJSON_GetStringValue(cJSON_GetObjectItem(root, "username"));
        memcpy(mqttConfig->username, username, strlen(username));

        char *password = cJSON_GetStringValue(cJSON_GetObjectItem(root, "password"));
        memcpy(mqttConfig->password, password, strlen(password));

        char *publish = cJSON_GetStringValue(cJSON_GetObjectItem(root, "publish"));
        memcpy(mqttConfig->publish, publish, strlen(publish));
        
        char *subscribe = cJSON_GetStringValue(cJSON_GetObjectItem(root, "subscribe"));
        memcpy(mqttConfig->subscribe, subscribe, strlen(subscribe));

        mqttConfig->port = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "port"));
        mqttConfig->version = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "version"));      
    }
    free(root);
}

static int get_serial_param(serialconfig_t *serialConfig)
{
    memset(read_buf,0,512);
    int read_ret = oc_read_file(SERIAL_CONFIG_FILE, read_buf);
    if(read_ret < 0){
		return -1;
	}

    cJSON *root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        serialConfig->baudrate = (unsigned long)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_baud"));
        serialConfig->databits = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_data"));
        serialConfig->stopbits = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_stop"));
        serialConfig->parity = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "serialSeting_Parity"));      
    }
    free(root);
    return 0;
}

static int get_socket_param(socketconfig_t *socketConfig)
{
    memset(read_buf,0,512);
    cJSON *root = NULL;
    int read_ret = oc_read_file("sockerconfig.json1", read_buf);
    if(read_ret < 0){
		return;
	}
    root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        socketConfig->tcpport = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "ip_port"));
        char *ipaddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "ip_address"));
        memcpy(socketConfig->tcpaddress, ipaddress, strlen(ipaddress));
    }

    memset(read_buf, 0 ,512);
    read_ret = oc_read_file("sockerconfig.json2", read_buf);
    if(read_ret < 0){
		return;
	}
    root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        socketConfig->udpport = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "ip_port"));
        char *ipaddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "ip_address"));
        memcpy(socketConfig->udpaddress, ipaddress, strlen(ipaddress));
    }
    free(root);
}

static int get_ali_param(aliconfig_t *aliConfig)
{
    memset(read_buf, 0 ,512);
    int read_ret = oc_read_file(ALI_CONFIG_FILE, read_buf);
    if(read_ret < 0){
		return;
	}

    cJSON *root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        char *product_key = cJSON_GetStringValue(cJSON_GetObjectItem(root, "product_key"));
        memcpy(aliConfig->product_key, product_key, strlen(product_key));
        char *product_secret = cJSON_GetStringValue(cJSON_GetObjectItem(root, "product_secret"));
        memcpy(aliConfig->product_secret, product_secret, strlen(product_secret));
        char *device_name = cJSON_GetStringValue(cJSON_GetObjectItem(root, "device_name"));
        memcpy(aliConfig->device_name, device_name, strlen(device_name));
        char *device_secret = cJSON_GetStringValue(cJSON_GetObjectItem(root, "device_secret"));
        memcpy(aliConfig->device_secret, device_secret, strlen(device_secret));
        aliConfig->puback_mode = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "puback_mode"));    
    }
    free(root);
}

static int get_passthrough_param()
{
    memset(read_buf, 0 ,512);
    int protocol = 0;
    int read_ret = oc_read_file(PASS_CONFIG_FILE, read_buf);
    if(read_ret <= 0){
		return -1;
	}

    cJSON *root = cJSON_ParseWithLength(read_buf, read_ret);
	if(root != NULL) {
        protocol = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(root, "passProtocol"));    
    }
    free(root);
    return protocol;
}

static void mqtt_param_init()
{
    cJSON *mqttroot = cJSON_CreateObject();
    cJSON_AddItemToObject(mqttroot, "clientid", cJSON_CreateString("nodered"));
    cJSON_AddItemToObject(mqttroot, "username", cJSON_CreateString("nodered"));
    cJSON_AddItemToObject(mqttroot, "password", cJSON_CreateString("nodered"));
    cJSON_AddItemToObject(mqttroot, "address", cJSON_CreateString("182.61.41.198"));
    cJSON_AddItemToObject(mqttroot, "port", cJSON_CreateNumber(1884));
    cJSON_AddItemToObject(mqttroot, "version", cJSON_CreateNumber(4));
    cJSON_AddItemToObject(mqttroot, "publish", cJSON_CreateString("v1/nodered/publish"));
    cJSON_AddItemToObject(mqttroot, "subscribe", cJSON_CreateString("v1/nodered/subscribe"));
    tool_mqtt_config_write(mqttroot);
    free(mqttroot);
    OC_UART_LOG_Printf("[%s] success!\n", __func__);
}

static void serial_param_init()
{
    char *buf = "{\"serialSeting_baud\":115200,\"serialSeting_data\":3, \"serialSeting_stop\":0, \"serialSeting_Parity\":0}";
    tool_serial_config_write(cJSON_ParseWithLength(buf, strlen(buf)));
    OC_UART_LOG_Printf("[%s] success!\n", __func__);
}

static void tcp_param_init()
{
    cJSON *tcproot = cJSON_CreateObject();
    cJSON_AddItemToObject(tcproot, "ip_address", cJSON_CreateString("182.61.41.198"));
    cJSON_AddItemToObject(tcproot, "ip_port", cJSON_CreateNumber(8012));
    cJSON_AddItemToObject(tcproot, "tcpudp", cJSON_CreateNumber(1));
    tool_tcp_config_write(tcproot);
    cJSON *udproot = cJSON_CreateObject();
    cJSON_AddItemToObject(udproot, "ip_address", cJSON_CreateString("182.61.41.198"));
    cJSON_AddItemToObject(udproot, "ip_port", cJSON_CreateNumber(8013));
    cJSON_AddItemToObject(udproot, "tcpudp", cJSON_CreateNumber(2));
    tool_tcp_config_write(udproot);
    free(tcproot);
    free(udproot);
    OC_UART_LOG_Printf("[%s] success!\n", __func__);
}

static void pass_param_init()
{
    cJSON *passroot = cJSON_CreateObject();
    cJSON_AddItemToObject(passroot, "passProtocol", cJSON_CreateNumber(TRNAS_THINGS));
    tool_pass_config_write(passroot);
    free(passroot);
    OC_UART_LOG_Printf("[%s] success!\n", __func__);
}

// 初次启动，创建配置文件
void device_config_init(dtu_config_t *currentConfig)
{
    // 判断是否是第一次启动，第一次启动创建文件，进入配置模式
    read_buf = (char *)malloc(512);
    memset(read_buf,0,512);
    int ret = oc_read_file(DTU_CONFIG_FILE, read_buf);
	if(ret <= 0){
        device_mode_write(CONFIG_MODE);
        serial_param_init();
        mqtt_param_init();
        tcp_param_init();
        pass_param_init();
    }

	if(get_serial_param(&currentConfig->currentserial) < 0){
		OC_UART_LOG_Printf("[%s] Get Serial param failed.\n", __func__);
	}
	if(get_mqtt_param(&currentConfig->currentmqtt) < 0){
		OC_UART_LOG_Printf("[%s] Get mqtt param failed.\n", __func__);
	}
	if(get_socket_param(&currentConfig->currentsocket) < 0){
		OC_UART_LOG_Printf("[%s] Get socket param failed.\n", __func__);
	}
	if(get_ali_param(&currentConfig->currentali) < 0){
		OC_UART_LOG_Printf("[%s] Get ali param failed.\n", __func__);
	}
    // 读取设备工作模式
	currentConfig->device_mode = get_device_mode();
    currentConfig->passthrougth = get_passthrough_param();
}