#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"
#include "dtu_common.h"

char *modbus_buf = "{\"slaves\":[{\"slave_address\":1,\"register_address\":3,\"function\":\"tem\",\"protocol\":1},{\"slave_address\":1,\"register_address\":4,\"function\":\"hum\",\"protocol\":1}]}";

int json_parse_file(void *slaves)
{
	cJSON *array = NULL;
	cJSON *item = NULL;
	unsigned int count = 0;
	
	cJSON *root = cJSON_Parse(modbus_buf);
	if(root == NULL) {
		return -1;
	}
		
	if(root != NULL) {
		array = cJSON_GetObjectItem(root, "slaves");
		if(cJSON_IsArray(array)){
			int array_size = cJSON_GetArraySize(array);
			OC_UART_LOG_Printf("array_size = %d\n", array_size);
			size_t i = 0;
			for (i = 0; i < array_size; i++)
			{
				if(i >= MAX_SLAVE){
					break;
				}
				slave_t *slave_item = (slave_t *)slaves;
				
				item = cJSON_GetArrayItem(array, i);
				slave_item->s_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "slave_address"));
				slave_item->r_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_address"));
				slave_item->protocol = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "protocol"));
				char *function = cJSON_GetStringValue(cJSON_GetObjectItem(item, "function"));
				memcpy(slave_item->function, function, strlen(function));
				OC_UART_LOG_Printf("%d,%d,%d,%s\n", slave_item->protocol, slave_item->s_address, slave_item->r_address, slave_item->function);
				slaves = slaves + sizeof(slave_t);
				count++;
			}
		}
	}
	
	free(root);
	free(array);
	free(item);
	return count;
}

void send_to_server(int procotol, char *message)
{
	switch (procotol)
	{
	case 1:
		OC_Mqtt_Publish("v1/devices/me/telemetry", 1, 0, message);
		break;

	case 2:
		// tcp
		break;
	
	default:
		break;
	}
}