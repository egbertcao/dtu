#include <stdio.h>
#include "global_types.h"
#include <stdlib.h>
#include "osa.h"
#include "modbus.h"
#include "oc_mqtt.h"
#include "cJSON.h"
#include "oc_uart.h"
#include "dtu_common.h"

slave_msg_t msg_buf[MAX_MSG];
dtu_config_t dtu_config;
static unsigned int byte_count = 0;
static char *g_serial_buf = NULL;
static Bool receivedFlag = FALSE;
unsigned int msg_count = 0;
unsigned int slave_ids[MAX_SLAVE];
unsigned int slave_count = 0;

static OSTaskRef dtuWorkerRef;
static OSTaskRef MasterWorkerRef;
ModBus_parameter* ModBus_Slave_paramater[MAX_SLAVE];

void dtu_readfromuart(char *buf_ptr, size_t size)
{	
	char buf[1024] = {0};
	if(strstr(buf_ptr, "SetDeviceMode:modbusMode")) {
		device_mode_write(MODBUS_MODE);
		return;
	}
	if(strstr(buf_ptr, "SetDeviceMode:configMode")) {
		device_mode_write(CONFIG_MODE);
		return;
	}
	if(strstr(buf_ptr, "SetDeviceMode:passthroughMode")) {
		device_mode_write(PASSTHROUGH_MODE);
		return;
	}
	int j = 0, i = 0;
	switch (dtu_config.device_mode)
	{
	case MODBUS_MODE:
		for(j = 0; j < size; j++){
			int i = 0;
			for(i=0; i<slave_count; i++){
				ModBus_readByteFromOuter(ModBus_Slave_paramater[slave_ids[i]], buf_ptr[j]);
			}
		}
		break;
	case CONFIG_MODE:
		for(i = 0; i < size; i++){
			if(buf_ptr[i] == '+'){
				receivedFlag = TRUE;
				break;
			}
			g_serial_buf[byte_count++] = buf_ptr[i];
		}
		if(receivedFlag){
			OC_UART_LOG_Printf("byte_count = %d\n", byte_count);
			OC_UART_LOG_Printf("[%s] %s\n", __func__, g_serial_buf);
			device_config(g_serial_buf, byte_count);
			memset(g_serial_buf, 0, 1024);
			byte_count = 0;
			receivedFlag = FALSE;
		}
		break;
	case PASSTHROUGH_MODE:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, buf_ptr, size);
		send_to_server(TRANS_MQTT , buf);
		break;
	
	default:
		break;
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

void modbus_work()
{
	memset(msg_buf, 0, MAX_SLAVE*sizeof(slave_msg_t));
	msg_count = get_modbus_slaves(msg_buf, slave_ids, &slave_count);
	if (msg_count <= 0) {
		OC_UART_LOG_Printf("get_modbus_slaves error!\n");
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

void customer_app_dtu_main(void)
{
	// 设备参数初始化
	OC_UART_LOG_Printf("Device config init.\n");
	device_config_init();
	g_serial_buf = (char *)malloc(1024);
	memset(g_serial_buf, 0, 1024);
	
	// 读取设备工作模式
	dtu_config.device_mode = device_mode_read();
	OC_UART_LOG_Printf("dtu_config.device_mode = %d\n", dtu_config.device_mode);
	if(dtu_config.device_mode == MODBUS_MODE) {	
		OC_UART_LOG_Printf("device is in modbus mode.\n");
		modbus_work();
	}
}