#include "osa.h"
#include "diag.h"
#include "customer_MainApp.h"
#include "oc_hal_feature_config.h"
#include "oc_uart.h"

#define CUSTOMER_APP_STACK_SIZE        1024*20  //  HEZI - Increase stack size
#define CUSTOMER_APP_TASK_PRIORITY      		220
static OSTaskRef	customer_app_TaskRef;
static UINT8*	customer_app_TaskStack = NULL;
static OSASemaRef		customerAppSemaRef = 0;
OSAFlagRef	customer_app_FlagRef;

extern void customer_app_uart_demo(void);
extern void customer_alimqtt_autoAck_demo(void);
OSA_STATUS CustomerAppLockNoBlock(void)
{
    return OSASemaphoreAcquire(customerAppSemaRef, OSA_NO_SUSPEND);   
}

OSA_STATUS CustomerAppUnlock(void)
{
    return OSASemaphoreRelease(customerAppSemaRef);   
}

void Customer_App_Main(void *arg)
{
	OS_STATUS status;
		
	OC_UART_LOG_Printf("Customer_App_Main!\n");

	status = OSAFlagCreate(&customer_app_FlagRef);
	ASSERT(status == OS_SUCCESS);

	status = OSASemaphoreCreate(&customerAppSemaRef, 1, OSA_FIFO);
	ASSERT(status == OS_SUCCESS);

	

	#if 1
	OC_UART_LOG_Printf("dtu main start!\n");
	customer_app_dtu_main();

	OC_UART_LOG_Printf("uart Start!\n");
	customer_app_uart_demo();

	OC_UART_LOG_Printf("mqtt Start!\n");
	customer_app_mqtt_demo();

	#endif			
}
