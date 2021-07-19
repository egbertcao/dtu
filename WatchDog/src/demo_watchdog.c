#include "oc_hal_feature_config.h"
#define OC_WATCHDOG_DEMO_MODULE_ENABLED 
#ifdef OC_WATCHDOG_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "oc_watchdog.h"

static void WatchdogInterruptRoutine(void)
{
	OC_UART_LOG_Printf("---WatchdogInterruptRoutine ---\n");
}
void customer_app_watchdog_demo(void)
{
	OC_WATCHDOG_Configuration wdgConfig;
	OC_WATCHDOG_ReturnCode    rc;

	//step1:set silent reset
	silentReset_Save();
	 
	OC_WatchDogProtect(FALSE);

	//step2
	OC_WatchDogInit();

	//step3
	wdgConfig.matchValue =  1000;
	/* RESET mode after "timeMs" */
	wdgConfig.mode = OC_WATCHDOG_RESET_MODE;

	/* Interrupt mode: */
	//wdgConfig.mode = OC_WATCHDOG_INTERRUPT_MODE;
	
	rc = OC_WatchDogConfigure(&wdgConfig);
	//OC_WatchDogRegister(WatchdogInterruptRoutine);
	//step4
	if(rc == OC_WATCHDOG_RC_OK)
	{
		rc = OC_WatchDogActivate();
	}
	OC_UART_LOG_Printf("---customer_app_watchdog_demo, rc=%d ---\n", rc);
	OC_WatchDogProtect(TRUE);
}

#endif
