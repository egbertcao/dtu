#include "oc_hal_feature_config.h"
#ifdef OC_SLEEP_MANAGER_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "customer_MainApp.h"

void customer_app_sleep_demo(void)
{
  	OC_UART_LOG_Printf("-----customer_app_sleep_demo-----\n");
	//enter sleep
	//OC_SystemSleepEnable(TRUE);

	//keep wakeup
	//OC_SystemSleepEnable(FALSE);
}
#endif
