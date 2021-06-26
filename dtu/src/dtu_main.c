#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"
#include "dtu_common.h"

msg_t msg_buf[MAX_MSG];
dtu_config_t dtu_config;
unsigned int msg_count = 0;
unsigned int slave_ids[MAX_SLAVE];
unsigned int slave_count = 0;

static OSTaskRef dtuWorkerRef;
static OSTaskRef MasterWorkerRef;
ModBus_parameter* ModBus_Slave_paramater[MAX_SLAVE];

void modbus_readfromuart(byte buf_ptr)
{
	int i =0;
	for(i=0; i<slave_count; i++){
		ModBus_readByteFromOuter(ModBus_Slave_paramater[slave_ids[i]], buf_ptr);
	}
}

// 寄存器值缓冲区首地址, 寄存器个数
static void modbus_get_response(uint16_t s_address, uint16_t r_address, uint16_t *buf_address, uint16_t size)
{
	char message[20] = {0};
	unsigned int i = 0;
	for(i = 0; i < msg_count; i++){
		if(s_address == msg_buf[i].s_address) {
			if(r_address == msg_buf[i].r_address) {
				OC_UART_LOG_Printf("modbus_get_response(%d) : (0x%x)/(0x%x) %s = ", size, s_address, r_address, msg_buf[i].function);
				break;
			}
		}
	}
	sprintf(message, "{%s:%d}",msg_buf[i].function,*(buf_address));
	send_to_server(msg_buf[i].protocol, message);
	OC_UART_LOG_Printf("%s\n", message);
}

void dtu_worker_thread(void * argv)
{
	while (1)
	{
		unsigned int i = 0;
		for(i = 0; i < msg_count; i++){
			ModBus_getRegister(ModBus_Slave_paramater[msg_buf[i].s_address], msg_buf[i].r_address, msg_buf[i].count, modbus_get_response);
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
		for(i=0; i<slave_count; i++){
			ModBus_Master_loop(ModBus_Slave_paramater[slave_ids[i]]);
			OSATaskSleep(200);
		}
	}
}

void customer_app_dtu_main(void)
{
	// 最开始判断配置文件是否存在，若不存在则进入配置模式，若存在则读取配置文件，根据配置选择是否进入配置模式
	dtu_config.dtu_mode = 0;  // 配置模式
	init_config(&dtu_config);
	if(dtu_config.dtu_mode == 0) {
		init_modbus_config();
	}

	memset(msg_buf, 0, MAX_SLAVE*sizeof(msg_t));
	msg_count = json_parse_file(msg_buf, slave_ids, &slave_count);
	if (msg_count <= 0) {
		OC_UART_LOG_Printf("json_parse_file error!\n");
		return;
	}
	int i = 0;
	for(i = 0; i < msg_count; i++)
	{
		OC_UART_LOG_Printf("%d,%d,%d,%s\n", msg_buf[i].s_address, msg_buf[i].r_address, msg_buf[i].protocol, msg_buf[i].function);
	}
	
	for(i=0;i<slave_count; i++)
	{
		OC_UART_LOG_Printf("slave%d\n", slave_ids[i]);
	}

	for(i = 0; i < slave_count; i++) {
		ModBus_Slave_paramater[slave_ids[i]]=(ModBus_parameter*)malloc(sizeof(ModBus_parameter));
		if(ModBus_Slave_paramater[slave_ids[i]] == NULL){
			return;
		}
		ModBus_Setting_T setting;
		setting.address = slave_ids[i];
		setting.frameType = RTU;
		setting.baudRate = 115200;
		setting.register_access_limit = 1;
		setting.sendHandler = sendHandler;
		ModBus_setup(ModBus_Slave_paramater[slave_ids[i]], setting);
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