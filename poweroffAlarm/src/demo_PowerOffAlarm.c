#include "oc_hal_feature_config.h"
#include <stdio.h>
#include "global_types.h"
#include "oc_poweroffalarm.h"
#include "osa.h"
#include "oc_uart.h"
void customer_app_poweroffalarm_demo(void)
{
    t_rtc test_tm;
	UINT8 i=0;
	//2018-2-2 02:02:02 [FRI]
	test_tm.tm_sec  = 2;
	test_tm.tm_min  = 2;
	test_tm.tm_hour = 2;
	test_tm.tm_mday = 8;
	test_tm.tm_mon  = 9;
	test_tm.tm_year = 2020;
	test_tm.tm_wday = 5;

	//set current
	PMIC_RTC_SetTime(&test_tm,APP_OFFSET);
    //get current
	PMIC_RTC_GetTime(&test_tm,APP_OFFSET);
	//set alarm
	test_tm.tm_min += 5;
	PMIC_RTC_SetAlarm(&test_tm);

	//enable RTC alarm
	PMIC_RTC_EnableAlarm(TRUE);

	// Manual Operation for EVB
	// 1) delay for manual plug-out usb , to avoid EXTON_WU
	while(i++ < 5){
		OSATaskSleep(200);
		uart_printf("[%s] sleep [%d] ......\r\n",__func__,i);
	}
	//update APP_RTC to verify PMIC_RTC_Alarm_Update_Follow_RTC_SetTime() logic
	test_tm.tm_min -= 1;
	PMIC_RTC_SetTime(&test_tm,APP_OFFSET);

	//reset PMIC regs & SW power down
	PM812_RESET_ALL();
	uart_printf("[%s] going to SW_PDOWN \r\n",__func__);
	OSATaskSleep(100);

    SysRestartReasonSet('A');
	PM812_SW_PDOWN();
	
}

