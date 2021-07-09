#ifndef _DUT_COMMON_H_
#define _DUT_COMMON_H_
#include "oc_pcac_fs.h"
#include "cJSON.h"

typedef struct slave {
	unsigned int protocol;      //采用什么方式传递到服务器
	unsigned int function_code;	// 功能码
	unsigned int s_address;		// 从机地址
	unsigned int r_address;		// 寄存器地址
	unsigned int count;			// 寄存器个数
	char function[20];			// 功能定义
	char data_deal[20];			// 数据处理方式
} slave_msg_t;

typedef struct deviceinfo {
	char imei[20];
	char imsi[20];
	char cgmr[100];
	char iccid[100];
	char csq[100];
	char creg[100];
	char longitude[30];
    char latitude[30];
} deviceinfo_t;

typedef struct dtuconfig {
	unsigned int device_mode;
} dtu_config_t;

typedef struct mqttconfig {
	char clientid[20];
	char username[20];
	char password[20];
	char address[20];
	int port;
	int version;
} mqttconfig_t;

typedef struct serialconfig {
	unsigned int baudrate;
	unsigned int databits;
	unsigned int stopbits;
	unsigned int parity;
} serialconfig_t;

typedef struct socketconfig {
	char tcpaddress[20];
	unsigned int tcpport;
	char udpaddress[20];
	unsigned int udpport;
} socketconfig_t;

typedef struct aliconfig {
	char product_key[20];
	char product_secret[20];
	char device_name[20];
	char device_secret[20];
	unsigned int puback_mode;
} aliconfig_t;

enum Device_mode
{
	CONFIG_MODE = 1,
	MODBUS_MODE,
	PASSTHROUGH_MODE,
};

enum TransProto
{
	TRANS_SERIAL = 1,
	TRANS_MQTT,
	TRANS_TCP,
	TRANS_UDP,
	TRANS_HTTP,
	TRNAS_ALI,
};

enum SerialFunctionCode {
	SetDtuMode = 10,
	GetDtuMode,
	GetDeviceInfo,
	SetModbusConfig,
	GetModbusConfig,
	SetMqttConfig,
	GetMqttConfig,
	SetPassProtocol,
	GetPassProtocol,
	SetSerialSetting,
	GetSerialSetting,
	SetTcpSetting,
	GetTcpSetting,
	SetAliSetting,
	GetAliSetting
};


#define MAX_SLAVE 20
#define MAX_MSG 20

#define DTU_CONFIG_FILE "dtuConfig.json"
#define MODBUS_INFO_FILE "modbusinfo.json"
#define MQTT_CONFIG_FILE "mqttConfig.json"
#define PASS_CONFIG_FILE "passconfig.json"
#define SERIAL_CONFIG_FILE "serialconfig.json"
#define SOCKET_CONFIG_FILE "sockerconfig.json"
#define ALI_CONFIG_FILE "aliconfig.json"

extern int oc_write_file(char *filename, char *buf);
extern int oc_read_file(char *filename, char *buf);
extern void send_to_server(int procotol, char *message);
extern void device_info_get(deviceinfo_t *deviceinfo);
extern int get_device_mode();
extern void device_config_init();
extern int get_modbus_slaves(void *slaves, unsigned int *slave_ids, unsigned int *slave_count);
extern void device_config(char *serialdata, size_t size);
extern void device_mode_write(int deviceMode);
extern int get_passthrough_param();
extern int get_ali_param(aliconfig_t aliConfig);
extern int get_socket_param(socketconfig_t socketConfig);
extern int get_serial_param(serialconfig_t serialConfig);
extern int get_mqtt_param(mqttconfig_t mqttConfig);
extern void tool_mqtt_config_write(cJSON *msg);
extern void tool_pass_config_write(cJSON *msg);
extern void tool_serial_config_write(cJSON *msg);
extern void tool_tcp_config_write(cJSON *msg);

#endif