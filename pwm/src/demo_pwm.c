#include "oc_hal_feature_config.h"
#ifdef OC_PWM_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "oc_uart.h"
#include "oc_pwm.h"

void customer_app_pwm_demo(void)
{
	OC_PWM_CONFIGURE config;
	BOOL on=TRUE;
	
	//OC_CPUartLogPrintf("%s, enter",__func__);
	OC_UART_LOG_Printf("%s, enter\n",__func__);
	//A sample for PWM cycle=300ms && high:low=5:1
      // step1. set the selected pin to PWM function.
	OC_PWM_Init();
	
 	//step2. pwm config 
 	//select to set prescale to max 0x3f, Scaled counter clock frequency=32K/(0x3f+1)=500,so one Scaled counter clock period = 1000ms/500=2ms
 	//total_one_cycle_duration=300ms/2ms=150
	//high_one_cycle_duration = 150*(5/6)=130
	if(on)
	{
		config.clk_freq = OC_PWM_PSCLK_32K/64; //32000/64=500                    
		config.high_one_cycle_duration = 130;   
		config.total_one_cycle_duration = 150;  //expected 300ms as a pwm cycle, it should equel to 300ms/2ms=150
		config.clk_src = OC_PWM_CLK_SRC_32K;
	}
	else
	{
		config.clk_freq = OC_PWM_PSCLK_32K/64;
		config.high_one_cycle_duration = 0;
		config.total_one_cycle_duration = 150;  //expected 300ms as a pwm cycle, it should equel to 300ms/2ms=150
		config.clk_src = OC_PWM_CLK_SRC_32K;	
	}
	
	OC_PWM_Configure(&config);

	//enbale pwm
	if(on)
		OC_PWM_Enable();
	else
		OC_PWM_Disable();
		
	//OC_CPUartLogPrintf("%s, exit",__func__);
	OC_UART_LOG_Printf("%s, exit\n",__func__);
}
#endif
