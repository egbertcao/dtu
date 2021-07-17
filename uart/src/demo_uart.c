#include "oc_hal_feature_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "global_types.h"
#include "oc_gpio.h"
#include "oc_uart.h"
//#include "oc_alimqtt.h"
#include "oc_mqtt.h"
#include "osa.h"
#include "modbus.h"
#include "dtu_common.h"

OSATaskRef customerUartRxTaskRef;
UINT8* customerUartRxTaskStack;
extern OSAFlagRef customer_Uart_FlagRef;
extern dtu_config_t g_dtu_config;

static void demo_uart_callback(void *user_data, OC_UART_Event uart_evt)
{
	OC_UART_LOG_Printf("demo_uart_callback,event=%d \r\n", uart_evt);
	if (OC_UART3_EVENT_RX_ARRIVED == uart_evt){
	}
	else if (OC_UART3_EVENT_RX_OVERFLOW == uart_evt){

	}else if (OC_UART3_EVENT_TX_COMPLETE== uart_evt){

	}
	else if (OC_UART1_EVENT_RX_ARRIVED == uart_evt){
	}
	else if (OC_UART1_EVENT_RX_OVERFLOW == uart_evt){

	}else if (OC_UART1_EVENT_TX_COMPLETE== uart_evt){

	}
}

void uart_init()
{
	OC_UARTConfiguration *uartDemoConfiguration;
	uartDemoConfiguration =(OC_UARTConfiguration *)malloc(sizeof(OC_UARTConfiguration));
	uartDemoConfiguration->oc_baudRate = g_dtu_config.currentserial.baudrate;
	uartDemoConfiguration->oc_numDataBits = g_dtu_config.currentserial.databits;
	uartDemoConfiguration->oc_parityBitType = g_dtu_config.currentserial.parity;
	uartDemoConfiguration->oc_stopBits = g_dtu_config.currentserial.stopbits;
	uartDemoConfiguration->oc_flowControl = FALSE;
	uartDemoConfiguration->oc_auto_baud = FALSE;
	uartDemoConfiguration->callback = demo_uart_callback;
	OC_UART_Init(OC_UART_PORT_3, uartDemoConfiguration);
}

void customerUartRxTask(VOID *argv)
{
	int i;
	UINT32   event = 0;    
	UINT32 readsize, length;
	UINT8 *buf_ptr = NULL;
	
	while(1)
	{
		OSAFlagWait(customer_Uart_FlagRef, OC_UART_EVENT_FLAGS_MASK, OSA_FLAG_OR_CLEAR, &event, OSA_SUSPEND);

		switch (event)
		{
			case OC_UART3_EVENT_RX_ARRIVED:
				//OC_UART_LOG_Printf("OC_UART3_EVENT_RX_ARRIVED\n");
				length = OC_UART_Get_Available_Receive_Bytes(OC_UART_PORT_3);
				//OC_UART_LOG_Printf("length=%d\n",length);

				buf_ptr = (UINT8 *)malloc(length);
    			ASSERT(buf_ptr != NULL);
				memset(buf_ptr, 0, length);
				readsize = OC_UART_Receive(OC_UART_PORT_3, buf_ptr, length);

				dtu_readfromuart(buf_ptr, readsize);
				
				free(buf_ptr);
				buf_ptr = NULL;
				break;
			case OC_UART3_EVENT_RX_OVERFLOW:	
				OC_UART_LOG_Printf("OC_UART3_EVENT_RX_OVERFLOW\n");
				break;
			case OC_UART3_EVENT_TX_COMPLETE:	
				OC_UART_LOG_Printf("OC_UART3_EVENT_TX_COMPLETE\n");
				break;
			case OC_UART1_EVENT_RX_ARRIVED:
				OC_UART_LOG_Printf("OC_UART1_EVENT_RX_ARRIVED\n");
				length = OC_UART_Get_Available_Receive_Bytes(OC_UART_PORT_1_AT);
				OC_UART_LOG_Printf("length=%d\n",length);

				 buf_ptr = (UINT8 *)malloc(length);
    				ASSERT(buf_ptr != NULL);
				readsize = OC_UART_Receive(OC_UART_PORT_1_AT, buf_ptr, length);

				for (i = 0; i < readsize; i++)
				 	OC_UART_LOG_Printf("%c", buf_ptr[i]);
				OC_UART_LOG_Printf("\n");
				free(buf_ptr);
				buf_ptr = NULL;
				break;
			case OC_UART1_EVENT_RX_OVERFLOW:	
				OC_UART_LOG_Printf("OC_UART1_EVENT_RX_OVERFLOW\n");
				break;
			case OC_UART1_EVENT_TX_COMPLETE:	
				OC_UART_LOG_Printf("OC_UART1_EVENT_TX_COMPLETE\n");
				break;	
			default:
				break;
		}
	}
}

void customer_app_uart_start(void)
{
	OC_UART_LOG_Printf("[%s] uart Start!\n", __func__);
	char uartTestMsg [1024];
	UINT16 datalen;
	UINT16 lenToRead = sizeof(uartTestMsg) -1;
	UINT16 dataLength;
	char   *pbuf = NULL;

 	uart_init();

	//uart3 test write string
	memset(uartTestMsg, 0, sizeof(uartTestMsg));
	sprintf (uartTestMsg, "UART3: helloword");
	datalen=strlen(uartTestMsg);
	OC_UART_Send(OC_UART_PORT_3, uartTestMsg, datalen);
	OSATaskSleep(5);

	OS_STATUS status;
	OC_UART_LOG_Printf("Uart Start!\n");
	customerUartRxTaskStack = (void*)malloc(1024*4);
	status = OSATaskCreate(&customerUartRxTaskRef,
					customerUartRxTaskStack, 
					1024*4, 
					100, 
					"customerUartRxTask", 
					customerUartRxTask, 
					NULL);
	ASSERT(status == OS_SUCCESS);
}
