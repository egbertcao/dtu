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

void send_to_server(int procotol, char *message)
{
	int len = strlen(message);
	char buf[1024] = {0};
	memcpy(buf, message, len);
	buf[len] = '\n';
	OC_UART_LOG_Printf("[%s] %s\n", __func__, buf);
	switch (procotol)
	{
	case TRANS_MQTT:
		OC_Mqtt_Publish("v1/gateway/telemetry", 1, 0, buf);
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