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
static unsigned int byte_count = 0;
static char *g_serial_buf = NULL;
static Bool receivedFlag = FALSE;
unsigned int msg_count = 0;
unsigned int slave_ids[MAX_SLAVE];
unsigned int slave_count = 0;

dtu_config_t g_dtu_config;

static OSTaskRef dtuWorkerRef;
static OSTaskRef MasterWorkerRef;
static ModBus_parameter* ModBus_Slave_paramater[MAX_SLAVE];

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
	switch (g_dtu_config.device_mode)
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
			if(buf_ptr[i] == '$'){
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
		send_to_server_pass(buf);
		break;
	
	default:
		break;
	}
}

// 寄存器值缓冲区首地址, 寄存器个数
static void modbus_get_response(uint16_t s_address, uint16_t r_address, uint16_t *buf_address, uint16_t size)
{
	unsigned int i = 0;
	unsigned int j = 0;
	for(i = 0; i < msg_count; i++){
		if(s_address == msg_buf[i].s_address) {
			if(r_address == msg_buf[i].r_address) {
				// 根据大小端模式处理数据
				unsigned int received_data = 0;
				if(size == 2) {
					if(msg_buf[i].endian == 1){
						unsigned short a = *(buf_address);
						unsigned short b = *(buf_address+1);
						received_data = (a << 16) + b;
					}
					if(msg_buf[i].endian == 0){
						unsigned short a = *(buf_address);
						unsigned short b = *(buf_address+1);
						received_data = (b << 16) + a;
					}
					OC_UART_LOG_Printf("[%s] received_data = 0x%x\n", __func__, received_data);
				}
				else {
					received_data = *(buf_address);
				}
				// 组包并发送
				send_to_server_modbus(s_address, msg_buf[i].function, msg_buf[i].multiply, received_data);
				break;
			}
		}
	}
}

void dtu_worker_thread(void * argv)
{
	while (1)
	{
		unsigned int i = 0;
		for(i = 0; i < msg_count; i++){
			ModBus_getRegister(ModBus_Slave_paramater[msg_buf[i].s_address], msg_buf[i].r_address, msg_buf[i].count, modbus_get_response);
			OC_UART_LOG_Printf("[%s] %d, %d, %d\n", __func__, msg_buf[i].s_address, msg_buf[i].r_address, msg_buf[i].count);
			OSATaskSleep(500);
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
	
	for(i = 0; i < slave_count; i++) {
		ModBus_Slave_paramater[slave_ids[i]]=(ModBus_parameter*)malloc(sizeof(ModBus_parameter));
		if(ModBus_Slave_paramater[slave_ids[i]] == NULL){
			return;
		}
		ModBus_Setting_T setting;
		setting.address = slave_ids[i];
		setting.frameType = RTU;
		setting.baudRate = g_dtu_config.currentserial.baudrate;
		setting.register_access_limit = 6; // 最多读取8个字节
		setting.sendHandler = sendHandler;
		ModBus_setBitRate(ModBus_Slave_paramater[slave_ids[i]], g_dtu_config.currentserial.baudrate);
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
	// 获取设备配置参数
	OC_UART_LOG_Printf("Device config init.\n");
	memset(&g_dtu_config, 0, sizeof(dtu_config_t));
	device_config_init(&g_dtu_config);

	OC_UART_LOG_Printf("[%s] -----------serialConfig----------------\n", __func__);
    OC_UART_LOG_Printf("[%s] baudrate = %lu\n", __func__, g_dtu_config.currentserial.baudrate);
	OC_UART_LOG_Printf("[%s] databits = %d\n", __func__, g_dtu_config.currentserial.databits);
	OC_UART_LOG_Printf("[%s] parity = %d\n", __func__, g_dtu_config.currentserial.parity);
	OC_UART_LOG_Printf("[%s] stopbits = %d\n", __func__, g_dtu_config.currentserial.stopbits);

	OC_UART_LOG_Printf("[%s] -----------mqttConfig----------------\n", __func__);
	OC_UART_LOG_Printf("[%s]: clientid = %s\n", __func__, g_dtu_config.currentmqtt.clientid);
	OC_UART_LOG_Printf("[%s]: username = %s\n", __func__, g_dtu_config.currentmqtt.username);
	OC_UART_LOG_Printf("[%s]: password = %s\n", __func__, g_dtu_config.currentmqtt.password);
	OC_UART_LOG_Printf("[%s]: address = %s\n", __func__, g_dtu_config.currentmqtt.address);
	OC_UART_LOG_Printf("[%s]: port = %d\n", __func__, g_dtu_config.currentmqtt.port);
	OC_UART_LOG_Printf("[%s]: version = %d\n", __func__, g_dtu_config.currentmqtt.version);
	OC_UART_LOG_Printf("[%s]: publish = %s\n", __func__, g_dtu_config.currentmqtt.publish);
	OC_UART_LOG_Printf("[%s]: subscribe = %s\n", __func__, g_dtu_config.currentmqtt.subscribe);
	
	OC_UART_LOG_Printf("[%s] -----------deviceMode----------------\n", __func__);
	OC_UART_LOG_Printf("device_mode = %d\n", g_dtu_config.device_mode);

	OC_UART_LOG_Printf("[%s] -----------passthrougth----------------\n", __func__);
	OC_UART_LOG_Printf("passthrougth = %d\n", g_dtu_config.passthrougth);

	g_serial_buf = (char *)malloc(1024);
	if(g_serial_buf == NULL) {
		return;
	}
	memset(g_serial_buf, 0, 1024);

	OC_UART_LOG_Printf("uart Start!\n");
	customer_app_uart_demo();
	dtu_netopen_worker();  // 阻塞等待网络连接

	if(g_dtu_config.passthrougth == TRANS_MQTT || g_dtu_config.passthrougth == TRNAS_THINGS){
		OC_UART_LOG_Printf("mqtt Start!\n");
		customer_app_mqtt_demo();
	}
	else if(g_dtu_config.passthrougth == TRANS_TCP){
		//tcp
	}
	else if(g_dtu_config.passthrougth == TRANS_UDP){
		// udp
	}
	else if(g_dtu_config.passthrougth == TRANS_HTTP){
		// http
	}
	else if(g_dtu_config.passthrougth == TRNAS_ALI){
		// ali
	}
	else{
		return;
	}
	
	OSATaskSleep(1000);  // 等待5s，连接服务器
	if(g_dtu_config.device_mode == MODBUS_MODE) {	
		OC_UART_LOG_Printf("device is in modbus mode.\n");
		modbus_work();
	}
}