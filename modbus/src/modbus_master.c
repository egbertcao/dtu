#include <stdio.h>
#include "global_types.h"
#include "osa.h"
#include "oc_uart.h"
#include "modbus.h"
#define MAX_SLAVE 20

static OSTaskRef MasterWorkerRef;
ModBus_parameter* ModBus_Slave_paramater[MAX_SLAVE];

void modbus_readfromuart(byte buf_ptr)
{
	ModBus_readByteFromOuter(ModBus_Slave_paramater[0], buf_ptr);
}

static void sendHandler(byte* data, size_t length){
	OC_UART_Send(OC_UART_PORT_3, data, length);
}

static void modbus_master(void *parameter)
{
	while (1)
	{
		ModBus_Master_loop(ModBus_Slave_paramater[0]);
		OSATaskSleep(200);
	}
}

void modbus_slave_init(ModBus_Setting_T setting)
{
	void *MasterTaskStack;
	MasterTaskStack=malloc(4096);
	if(MasterTaskStack == NULL)
	{
		return;
	}

	setting.sendHandler = sendHandler;
	ModBus_setup(ModBus_Slave_paramater[0], setting);
	
	if(OSATaskCreate(&MasterWorkerRef,
	                 MasterTaskStack,
	                 4096,80,(char*)"PollTask",
	                 modbus_master, NULL) != 0)
	{
		return;
	}

}


