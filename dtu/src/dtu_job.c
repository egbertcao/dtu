#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"

#define MAX_SLAVE 20
static OSTaskRef dtuWorkerRef;
extern ModBus_parameter* ModBus_Slave_paramater[MAX_SLAVE];

char *modbus_buf = "{\"slaves\":[{\"slave_address\":1,\"register_address\":3,\"function\":\"tem\",\"protocol\":1},{\"slave_address\":1,\"register_address\":4,\"function\":\"hum\",\"protocol\":1}]}";
typedef struct slave {
	unsigned int protocol;
	unsigned int s_address;
	unsigned int r_address;
	char function[10];
} slave_t;

slave_t slaves[MAX_SLAVE];
int slave_count = 0;

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

// 寄存器值缓冲区首地址, 寄存器个数
static void modbus_get_response(uint16_t *address, uint16_t size)
{
	char message[20] = {0};
	unsigned int i = 0;
	OC_UART_LOG_Printf("modbus_get_response size = %d\n", size);
	for(i = 0; i < size; i++){
		OC_UART_LOG_Printf("%x ", *(address+i));
	}
	OC_UART_LOG_Printf("\n");
}

void dtu_worker_thread(void * argv)
{
	memset(slaves, 0, MAX_SLAVE*sizeof(slave_t));
	slave_count = json_parse_file(slaves);
	if (slave_count <= 0) {
		return;
	}
	
	while (1)
	{
		unsigned int i = 0;
		for(i = 0; i < slave_count; i++){
			OC_UART_LOG_Printf("slave %d: address %d\n", slaves[i].s_address, slaves[i].r_address);
			ModBus_getRegister(ModBus_Slave_paramater[0], slaves[i].r_address, 1, modbus_get_response);
			OSATaskSleep(2000);
		}
	}
}

void customer_app_dtu_main(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}

	// create modbus slave depends on slave instance
	OC_UART_LOG_Printf("modbus Start!\n");
	modbus_app();

	if(OSATaskCreate(&dtuWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"dtuOpenTask",
	                 dtu_worker_thread, NULL) != 0)
	{
		return;
	}
}