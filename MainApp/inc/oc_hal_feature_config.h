#ifndef __OC_HAL_FEATURE_CONFIG_H__
#define __OC_HAL_FEATURE_CONFIG_H__
/*****************************************************************************
* module ON or OFF feature option,only option in this temple
*****************************************************************************/
#ifdef OPENCPU_SDK
#define OC_HAL_ADC_MODULE_ENABLED	0
#define OC_HAL_EINT_MODULE_ENABLED	0
#define OC_HAL_GPIO_MODULE_ENABLED	0
#define OC_HAL_I2C_MASTER_MODULE_ENABLED	0      
#define OC_HAL_UART_MODULE_ENABLED	0
#define OC_HAL_SLEEP_MANAGER_ENABLED	0
#define OC_HAL_SPI_MASTER_MODULE_ENABLED	0
#define OC_HAL_WATCHDOG_MODULE_ENABLED		0
#define OC_HAL_PWM_MODULE_ENABLED	0
#define OC_HAL_AUDIO_MODULE_ENABLED		0
#else
#define OC_HAL_ADC_MODULE_ENABLED	1
#define OC_HAL_EINT_MODULE_ENABLED	1
#define OC_HAL_GPIO_MODULE_ENABLED	1
#define OC_HAL_I2C_MASTER_MODULE_ENABLED	1      
#define OC_HAL_UART_MODULE_ENABLED	1
#define OC_HAL_SLEEP_MANAGER_ENABLED	1
#define OC_HAL_SPI_MASTER_MODULE_ENABLED	0
#define OC_HAL_WATCHDOG_MODULE_ENABLED		1
#define OC_HAL_PWM_MODULE_ENABLED	1
#define OC_HAL_AUDIO_MODULE_ENABLED		1
#endif
/*****************************************************************************
 * * DEMO module feature option
 * *****************************************************************************/
#if OC_HAL_ADC_MODULE_ENABLED
#define OC_ADC_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_EINT_MODULE_ENABLED
#define OC_EINT_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_GPIO_MODULE_ENABLED
#define OC_GPIO_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_I2C_MASTER_MODULE_ENABLED
#define OC_I2C_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_SPI_MASTER_MODULE_ENABLED
#define OC_SPI_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_UART_MODULE_ENABLED
#define OC_UART_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_SLEEP_MANAGER_ENABLED
#define OC_SLEEP_MANAGER_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_WATCHDOG_MODULE_ENABLED
//#define OC_WATCHDOG_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_PWM_MODULE_ENABLED
#define OC_PWM_DEMO_MODULE_ENABLED
#endif
#if OC_HAL_AUDIO_MODULE_ENABLED
#define OC_AUDIO_DEMO_MODULE_ENABLED
#endif
/*****************************************************************************
* module ON or OFF feature option,only option in this temple
*****************************************************************************/
#endif /* __OC_HAL_FEATURE_CONFIG_H__*/

