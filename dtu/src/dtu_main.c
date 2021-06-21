#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"
#include "dtu_common.h"

slave_t slaves[MAX_SLAVE];
int slave_count = 0;

static OSTaskRef dtuWorkerRef;
static OSTaskRef MasterWorkerRef;
ModBus_parameter* ModBus_Slave_paramater[MAX_SLAVE];

void modbus_readfromuart(byte buf_ptr)
{
	int i =0;
	for(i=0; i<2; i++){
		ModBus_readByteFromOuter(ModBus_Slave_paramater[i], buf_ptr);
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
		for(i = 0; i < 2; i++){
			ModBus_getRegister(ModBus_Slave_paramater[i], slaves[i].r_address, 1, modbus_get_response);
			OSATaskSleep(2000);
		}
	}
}

// modbus发送接口
static void sendHandler(byte* data, size_t length){
	OC_UART_Send(OC_UART_PORT_3, data, length);
}


static void modbus_master(void *parameter)
{
	while (1)
	{
		int i = 0;
		for(i=0; i<2; i++){
			ModBus_Master_loop(ModBus_Slave_paramater[i]);
			OSATaskSleep(200);
		}
	}
}

void customer_app_dtu_main(void)
{
	int i = 0;
	for(i=0; i<2; i++) {
		ModBus_Slave_paramater[i]=(ModBus_parameter*)malloc(sizeof(ModBus_parameter));
		if(ModBus_Slave_paramater[i] == NULL){
			return;
		}
		ModBus_Setting_T setting;
		setting.address = i+3;
		setting.frameType = RTU;
		setting.baudRate = 115200;
		setting.register_access_limit = 1;
		setting.sendHandler = sendHandler;
		ModBus_setup(ModBus_Slave_paramater[i], setting);
	}

	void *MasterTaskStack;
	MasterTaskStack=malloc(4096);
	if(MasterTaskStack == NULL){
		return;
	}

	if(OSATaskCreate(&MasterWorkerRef,
	                 MasterTaskStack,
	                 4096,80,(char*)"PollTask",
	                 modbus_master, NULL) != 0){
		return;
	}

	void *DtuTaskStack;
	DtuTaskStack=malloc(4096);
	if(DtuTaskStack == NULL){
		return;
	}
	if(OSATaskCreate(&dtuWorkerRef,
	                 DtuTaskStack,
	                 4096,80,(char*)"dtuOpenTask",
	                 dtu_worker_thread, NULL) != 0){
		return;
	}
}