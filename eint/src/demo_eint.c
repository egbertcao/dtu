#include "oc_hal_feature_config.h"
#ifdef OC_EINT_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "oc_gpio.h"
#include "osa.h"
#include "customer_MainApp.h"

#define CUSTOM_SYS_DETECT_DEBOUNCE_TIME_TICK             4 /* ticks */
typedef enum
{
    CUSTOM_SYS_DETECT_RUN_HW_EVENT,
    CUSTOM_SYS_DETECT_RUN_SW_DEBOUNCE_POLLING,
    CUSTOM_SYS_DETECT_DONE,
    CUSTOM_SYS_DETECT_MAX          
} CustomSysDetectStateEnum;

CustomSysDetectStateEnum customSysDetectState;
OSATimerRef TimerEnableCustomEintDemoED;
static OS_HISR customWakeupSysHISR = NULL;
static OS_HISR customEnterSleepHISR = NULL;
static OS_HISR customEintDemoHISR = NULL;
OC_GPIORC_PIN_NUM custom_demo_eint_pin = OC_GPIORC_PIN_GPIO7;
OC_GPIORC_PINMUX_FUNCTION custom_demo_eint_pin_mode = OC_GPIORC_PIN_GPIO7_M_EINT;
extern OSAFlagRef	customer_app_FlagRef;
//Function
void customEnableSysEdgeDetectTimer(UINT32 id)
{
	volatile unsigned int data=0;
	OS_STATUS os_status;
	UINT8 event;
	//fatal_printf("EnableSystemWakeupEdgeDetectTimer\n");	
	data = OC_GpioGetLevel(custom_demo_eint_pin);
	if (data)
		event = CUSTOMER_APP_SYS_SLEEP_EVENT;
	else
		event = CUSTOMER_APP_SYS_WAKEUP_EVENT;
	 customSysDetectState = CUSTOM_SYS_DETECT_DONE;
	os_status = OSAFlagSet(customer_app_FlagRef, event, OSA_FLAG_OR);
	ASSERT(os_status==OS_SUCCESS);	
	OC_GpioEnableEdgeDetection(custom_demo_eint_pin,OC_GPIO_TWO_EDGE);					
}
static void custom_eint_demo_hisr(void)
{
    if(customSysDetectState == CUSTOM_SYS_DETECT_RUN_SW_DEBOUNCE_POLLING)
        OSATimerStop(TimerEnableCustomEintDemoED);
    customSysDetectState = CUSTOM_SYS_DETECT_RUN_SW_DEBOUNCE_POLLING;
    OSATimerStart(TimerEnableCustomEintDemoED, CUSTOM_SYS_DETECT_DEBOUNCE_TIME_TICK, 0, customEnableSysEdgeDetectTimer, 0);					
}
void customEintDemoDetectISR(void)
{
	 STATUS status;
	OC_GpioDisableEdgeDetection(custom_demo_eint_pin, OC_GPIO_TWO_EDGE);	
	customSysDetectState = CUSTOM_SYS_DETECT_RUN_HW_EVENT;
   	status = OS_Activate_HISR(&customEintDemoHISR);
    	ASSERT(status == OS_SUCCESS);
}

void customEintDemoDetectWakeUp(void)
{
	volatile unsigned int data;
	data = OC_GpioGetLevel(custom_demo_eint_pin);
	OC_UART_LOG_Printf("customEintDemoDetectWakeUp level:%x\n", data);
	/* Activate HISR*/
}
void custom_interrupt_config(OC_GPIORC_PIN_NUM gpio_pin)
{
	OC_GPIOConfiguration customEintConfig;
	OC_GPIOReturnCode ret;

	customEintConfig.pinDir = OC_GPIO_DIRECTION_INPUT;
	customEintConfig.pinEd = OC_GPIO_NO_EDGE;
	customEintConfig.pinPull = OC_GPIO_PULLUP_ENABLE;//OC_GPIO_PULL_DISABLE
	customEintConfig.initLv = OC_GpioGetLevel(gpio_pin);
	customEintConfig.isr = customEintDemoDetectISR;
	customEintConfig.wu=customEintDemoDetectWakeUp;
	ret = OC_GpioInterruptInitConfiguration(gpio_pin, customEintConfig);
	
    	OC_UART_LOG_Printf("custom_interrupt_config,ret=%d, LVL=%d\n", ret, OC_GpioGetLevel(gpio_pin));
	// clear INT	
	OC_GpioClearEdgeDetection(gpio_pin);
	//enable INT
	OC_GpioEnableEdgeDetection(gpio_pin, OC_GPIO_TWO_EDGE);	
}

void customer_app_eint_demo(void)
{
	UINT8 gpioNum;
	
  	OC_UART_LOG_Printf("customer_app_eint_demo\n");
	//step1: create hisr & timer
	OS_Create_HISR(&customEintDemoHISR, "customEintDemoHISR", custom_eint_demo_hisr,HISR_PRIORITY_2);
	OSATimerCreate(&TimerEnableCustomEintDemoED);

	//step2: eint config & register
	OC_GpioSetFunction(custom_demo_eint_pin, custom_demo_eint_pin_mode);
 	custom_interrupt_config(custom_demo_eint_pin);
}
#endif

