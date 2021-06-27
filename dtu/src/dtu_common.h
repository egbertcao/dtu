#ifndef _DUT_COMMON_H_
#define _DUT_COMMON_H_

typedef struct slave {
	unsigned int protocol;
	unsigned int s_address;
	unsigned int r_address;
	unsigned int count;
	char function[10];
} msg_t;

typedef struct dtuconfig {
	unsigned int device_mode;
} dtu_config_t;


enum Device_mode
{
	CONFIG_MODE,
	MODBUS_MODE,
};


#define MAX_SLAVE 20
#define MAX_MSG 20


#define MODBUS_JSON_FILE "modbusConfig.json"
#define DTU_CONFIG_FILE "dtuConfig.json"

#endif