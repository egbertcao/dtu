#include "oc_hal_feature_config.h"
#ifdef OC_ADC_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "oc_uart.h"
#include "oc_adc.h"

void customer_app_adc_demo(void)
{
	UINT32 adc_voltage;
	
	adc_voltage = OC_ADC_Get_Data(OC_ADC_CHANNEL_0);
	//CPUartLogPrintf("customer_app_adc_demo--adc_voltage=%d!\r\n",adc_voltage);
	OC_UART_LOG_Printf("customer_app_adc_demo--adc_voltage=%d mv!\r\n",adc_voltage);
}
#endif