#include "oc_hal_feature_config.h"
#ifdef OC_I2C_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "oc_gpio.h"

extern int codec_nau8814_check_device_id(void);
void customer_app_i2c_demo(void)
{
  	OC_UART_LOG_Printf("-----customer_app_i2c_demo-----\n");
	codec_nau8814_check_device_id();
}
#endif