#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"
#include "dtu_common.h"

extern dtu_config_t g_dtu_config;

void received_from_server(char *buf, int len, int procotol)
{
	OC_UART_LOG_Printf("[%s] received = %s.\n",__func__, buf);
	cJSON *root = NULL;
	cJSON *item = NULL;
	char message_buf[1024] = {0};
	switch (g_dtu_config.device_mode)
	{
	case MODBUS_MODE:
		// {"method":"SetModbus", "params":{"s_address": 0, "r_address":1, "count":2, "data":123}}
		root = cJSON_ParseWithLength(buf, len);
		if(!cJSON_IsObject(root)){
			OC_UART_LOG_Printf("[%s] received data is not json.\n",__func__);
			return;
		}
		item = cJSON_GetObjectItem(root, "params");
		if(!cJSON_IsObject(item)){
			OC_UART_LOG_Printf("[%s] received params is not json.\n",__func__);
			return;
		}
		uint16_t s_address = cJSON_GetNumberValue(cJSON_GetObjectItem(item, "s_address"));
		uint16_t r_address = cJSON_GetNumberValue(cJSON_GetObjectItem(item, "r_address"));
		uint16_t data = cJSON_GetNumberValue(cJSON_GetObjectItem(item, "data"));
		uint16_t count = cJSON_GetNumberValue(cJSON_GetObjectItem(item, "count"));
		modbus_set_data(s_address, r_address, data, count);
		break;
	case PASSTHROUGH_MODE:
		memcpy(message_buf, buf, len);	
		message_buf[len] = '\n';
		OC_UART_Send(OC_UART_PORT_3, message_buf, len+1);
		break;
	default:
		break;
	}

	if(root != NULL){
		free(root);
	}
	if(item != NULL){
		free(item);
	}
}

void send_to_server_modbus(uint16_t s_address, char *functionCode, int multiply, unsigned int received_data)
{
    if(g_dtu_config.device_mode != MODBUS_MODE) {
        return;
    }
	int bRet = 0;
    char message_buf[200] = {0};
	float received_float = 0;
    switch (multiply)
    {
    case 1:
        received_float = received_data * 0.001;
        break;
    case 2:
        received_float = received_data * 0.01;
        break;
    case 3:
        received_float = received_data * 0.1;
        break;
    case 4:
        break;
    case 5:
        received_float = received_data * 10;
        break;
    case 6:
        received_float = received_data * 100;
        break;
    case 7:
        received_float = received_data * 1000;
        break;
    
    default:
        break;
    }
    sprintf(message_buf, "{\"device%d\":[{\"%s\":%0.4f}]}",s_address, functionCode, received_float);
	OC_UART_LOG_Printf("[%s] %s\n", __func__, message_buf);
	cJSON *root = cJSON_Parse(message_buf);
	if(!cJSON_IsObject(root)){
		OC_UART_LOG_Printf("[%s] serialdata is not json.\n",__func__);
		return;
	}
	switch (g_dtu_config.passthrougth)
	{
	case TRNAS_THINGS:
        if(2 == OC_Mqtt_State()){
            bRet = OC_Mqtt_Publish("v1/gateway/telemetry", 0, 0, message_buf);
		    OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
        }
		break;
	case TRANS_MQTT:
        if(2 == OC_Mqtt_State()){
            OC_UART_LOG_Printf("[%s] %s\n", __func__, g_dtu_config.currentmqtt.publish);
            bRet = OC_Mqtt_Publish(g_dtu_config.currentmqtt.publish, 0, 0, message_buf);
            OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
        }
		break;
	case TRANS_TCP:
		dtu_tcp_send(message_buf);
		break;
	case TRANS_HTTP:
		dtu_http_send(message_buf);
		break;

	default:
		break;
	}
}

void send_to_server_pass(char *message)
{
    if(g_dtu_config.device_mode != PASSTHROUGH_MODE) {
        return;
    }
	int bRet = 0;
	char message_buf[1024] = {0};
	sprintf(message_buf, "{\"msg\":%s}", message);
	switch (g_dtu_config.passthrougth)
	{
	case TRNAS_THINGS:
		break;
	case TRANS_MQTT:
	    if(2 == OC_Mqtt_State()){
            OC_UART_LOG_Printf("[%s] %s\n", __func__, g_dtu_config.currentmqtt.publish);
            bRet = OC_Mqtt_Publish(g_dtu_config.currentmqtt.publish, 0, 0, message_buf);
            OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
        }
		break;
	case TRANS_TCP:
		dtu_tcp_send(message_buf);
		break;
	case TRANS_UDP:
		dtu_udp_send(message_buf);
		break;
	
	default:
		break;
	}
}