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
	cJSON *array = NULL;
	cJSON *item = NULL;
	unsigned int msg_count = 0;
	unsigned int s_count = 0;

    char buf[1024] = {0};
	int ret = oc_read_file(MODBUS_JSON_FILE, buf);
	if(ret <= 0){
		return -1;
	}

	//cJSON *root = cJSON_Parse(modbus_buf);
	cJSON *root = cJSON_Parse(buf);
	if(root == NULL) {
		return -1;
	}
		
	if(root != NULL) {
		array = cJSON_GetObjectItem(root, "msg");
		if(cJSON_IsArray(array)){
			int array_size = cJSON_GetArraySize(array);
			printf("array_size = %d\n", array_size);
			size_t i = 0;
			for (i = 0; i < array_size; i++)
			{
				if(i >= MAX_SLAVE){
					break;
				}
				msg_t *slave_item = (msg_t *)slaves;
				
				item = cJSON_GetArrayItem(array, i);
				slave_item->s_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "slave_address"));
				slave_item->r_address = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "register_address"));
				slave_item->protocol = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "protocol"));
				slave_item->count = (unsigned short)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "count"));
				char *function = cJSON_GetStringValue(cJSON_GetObjectItem(item, "function"));
				memcpy(slave_item->function, function, strlen(function));
				slave_ids[i] = slave_item->s_address;
				s_count = s_count + 1;
				slaves = slaves + sizeof(msg_t);
				msg_count++;
			}
		}
	}
	
	free(root);
	free(array);
	free(item);
	
	arr_rpttn(slave_ids, s_count);
	int count = 0;
	int i = 0;
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