#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "oc_pcac_fs.h"
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"
#include "dtu_common.h"
extern dtu_config_t g_dtu_config;

void send_to_server(int procotol, char *message)
{
	int len = strlen(message);
	char buf[1024] = {0};
	int bRet = 0;
	memcpy(buf, message, len);
	buf[len] = '\n';
	OC_UART_LOG_Printf("[%s] %s\n", __func__, buf);
	switch (procotol)
	{
	case TRNAS_THINGS:
		bRet = OC_Mqtt_Publish("v1/gateway/telemetry", 0, 0, "{\"device1\":[{\"test1\":456}]}");
		OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
		break;
	case TRANS_MQTT:
		OC_UART_LOG_Printf("[%s] %s\n", __func__, g_dtu_config.currentmqtt.publish);
		bRet = OC_Mqtt_Publish(g_dtu_config.currentmqtt.publish, 0, 0, "{\"device1\":[{\"test1\":456}]}");
		OC_UART_LOG_Printf("[%s] send result = %d\n", __func__, bRet);
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