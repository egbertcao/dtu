#include "oc_hal_feature_config.h"
#ifdef OC_GPIO_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "oc_gpio.h"

void customer_app_gpio_demo(void)
{
	OC_GPIOPinDirection gpio_dir;
	OC_GPIOReturnCode gpio_data;
	
	OC_UART_LOG_Printf("-----GPIO USEAGE  Demo-----\n");

	//example:use OC_GPIORC_PIN_GPIO82
	OC_GpioSetFunction(OC_GPIORC_PIN_GPIO82, OC_GPIORC_PIN_GPIO82_M_GPIO);
	OC_GpioSetDirection(OC_GPIORC_PIN_GPIO82, OC_GPIO_DIRECTION_OUTPUT);
	OC_GpioSetLevel(OC_GPIORC_PIN_GPIO82, OC_GPIO_HIGH);
	OC_GpioSetPullControl(OC_GPIORC_PIN_GPIO82, OC_GPIO_PULL_DISABLE);
	gpio_dir = OC_GpioGetDirection(OC_GPIORC_PIN_GPIO82);
	gpio_data = OC_GpioGetLevel(OC_GPIORC_PIN_GPIO82);
	OC_UART_LOG_Printf("PIN%d: dir=%d, output=%d\n", OC_GPIORC_PIN_GPIO82, gpio_dir, gpio_data);
	OC_GpioSetLevel(OC_GPIORC_PIN_GPIO82, OC_GPIO_LOW);
	gpio_data = OC_GpioGetLevel(OC_GPIORC_PIN_GPIO82);
	OC_UART_LOG_Printf("PIN%d: output=%d\n", OC_GPIORC_PIN_GPIO82, gpio_data);
	OC_GpioSetDirection(OC_GPIORC_PIN_GPIO82, OC_GPIO_DIRECTION_INPUT);
	gpio_dir = OC_GpioGetDirection(OC_GPIORC_PIN_GPIO82);
	OC_UART_LOG_Printf("PIN%d: gpio_dir=%d\n", OC_GPIORC_PIN_GPIO82,  gpio_dir);

	OC_UART_LOG_Printf("-----GPIO USEAGE  Demo-----END!----\n");
	
}
#endif

