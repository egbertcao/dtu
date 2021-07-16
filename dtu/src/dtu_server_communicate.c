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

void received_from_server(char *buf, int len, int procotol, unsigned short pack_id)
{
	char requestid[100] = {0};
	switch (procotol)
	{
	case TRANS_MQTT:
		OC_UART_LOG_Printf("[%s] %s\n", __func__, buf);
		//ModBus_setRegister(buf);
		//sprintf(requestid, "%s%d", g_dtu_config.currentmqtt.publish, pack_id);
		//OC_Mqtt_Publish(requestid, 1, 0, buf);
		break;

	case TRANS_TCP:
		// tcp
		break;

	case TRANS_SERIAL:
		OC_UART_Send(OC_UART_PORT_3, buf, len+1);
		break;
	
	default:
		break;
	}
}

void send_to_server_modbus(uint16_t s_address, char *functionCode, int multiply, unsigned int received_data)
{
    if(g_dtu_config.device_mode != MODBUS_MODE) {
        return;
    }
	int bRet = 0;
    char message_buf[200] = {0};
    switch (multiply)
    {
    case 1:
        received_data = received_data * 0.001;
        break;
    case 2:
        received_data = received_data * 0.01;
        break;
    case 3:
        received_data = received_data * 0.1;
        break;
    case 4:
        break;
    case 5:
        received_data = received_data * 10;
        break;
    case 6:
        received_data = received_data * 100;
        break;
    case 7:
        received_data = received_data * 1000;
        break;
    
    default:
        break;
    }
    sprintf(message_buf, "{\"device%d\":[{\"%s\":%d}]}",s_address, functionCode, received_data);
	OC_UART_LOG_Printf("[%s] %s\n", __func__, message_buf);
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
		// tcp
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
	switch (g_dtu_config.passthrougth)
	{
	case TRNAS_THINGS:
		break;
	case TRANS_MQTT:
		break;

	case TRANS_TCP:
		// tcp
		break;
	
	default:
		break;
	}
}