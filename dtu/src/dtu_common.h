#ifndef _DUT_COMMON_H_
#define _DUT_COMMON_H_
#include "oc_pcac_fs.h"

typedef struct slave {
	unsigned int protocol;
	unsigned int s_address;
	unsigned int r_address;
	unsigned int count;
	char function[10];
} slave_msg_t;

typedef struct deviceinfo {
	char imei[20];
	char imsi[20];
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
#define MODBUS_INFO_FILE "modbusinfo"
#define MQTT_CONFIG_FILE "mqttConfig.json"

extern int oc_write_file(char *filename, char *buf);
extern int oc_read_file(char *filename, char *buf);
extern void send_to_server(int procotol, char *message);
extern void device_info_get(deviceinfo_t *deviceinfo);
extern int device_mode_read();
extern void device_config_init();
extern int get_modbus_slaves(void *slaves, unsigned int *slave_ids, unsigned int *slave_count);
extern void device_config(char *serialdata, size_t size);
extern void device_mode_write(int deviceMode);
#endif