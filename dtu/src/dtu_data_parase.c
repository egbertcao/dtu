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
						  
int json_parse_file(void *slaves, unsigned int *slave_ids, unsigned int *slave_count)
{
	cJSON *item = NULL;
	unsigned int msg_count = 0;
	unsigned int s_count = 0;
    char buf[20] = 0;
	char modbusinfo_buf[1024];
	char filename[20] = 0;
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
			slave_item->r_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_address"));
			slave_item->protocol = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "protocol"));
			slave_item->count = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "count"));
			char *function = cJSON_GetStringValue(cJSON_GetObjectItem(item, "function"));
			memcpy(slave_item->function, function, strlen(function));
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

void send_to_server(int procotol, char *message)
{
	switch (procotol)
	{
	case TRANS_MQTT:
		OC_Mqtt_Publish("v1/devices/me/telemetry", 1, 0, message);
		break;

	case TRANS_TCP:
		// tcp
		break;

	case TRANS_SERIAL:
		OC_UART_Send(OC_UART_PORT_3, message, strlen(message));
		break;
	
	default:
		break;
	}
}