#ifndef _DUT_COMMON_H_
#define _DUT_COMMON_H_
#include "oc_pcac_fs.h"
#include "cJSON.h"

typedef struct slave {
	unsigned int function_code;	// 功能码
	unsigned int s_address;		// 从机地址
	unsigned int r_address;		// 寄存器地址
	unsigned int count;			// 寄存器个数
	char function[20];			// 功能定义
	unsigned int multiply;      // 1 x0.001, 2 x0.01, 3 x0.1 ...
	unsigned int endian;			// 1 大端， 2 小端
} modbus_slave_msg_t;

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

typedef struct mqttconfig {
	char clientid[100];
	char username[100];
	char password[100];
	char address[100];
	char publish[100];
	char subscribe[100];
	int port;
	int version;
} mqttconfig_t;

typedef struct serialconfig {
	unsigned long baudrate;
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
	TRNAS_THINGS,
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

typedef struct dtuconfig {
	unsigned int device_mode;
	unsigned int passthrougth;
	mqttconfig_t currentmqtt;
	serialconfig_t currentserial;
	socketconfig_t currentsocket;
	aliconfig_t currentali;
	deviceinfo_t deviceinfo;
} dtu_config_t;


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
extern void device_info_get(deviceinfo_t *deviceinfo);
extern void device_config_init(dtu_config_t *currentConfig);
extern int get_modbus_slaves(void *slaves, unsigned int *slave_ids, unsigned int *slave_count);
extern void device_config(char *serialdata, size_t size);
extern void device_mode_write(int deviceMode);
extern void tool_mqtt_config_write(cJSON *msg);
extern void tool_pass_config_write(cJSON *msg);
extern void tool_serial_config_write(cJSON *msg);
extern void tool_tcp_config_write(cJSON *msg);
extern void received_from_server(char *buf, int len, int protocol);
extern void send_to_server_pass(char *message);
extern void send_to_server_modbus(unsigned short s_address, char *functionCode, int multiply,unsigned int received_data);
extern int dtu_netopen_worker();
extern void modbus_set_data(unsigned short s_address, unsigned short r_address, unsigned short data, unsigned short count);
#endif