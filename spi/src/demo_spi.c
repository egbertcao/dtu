#include "oc_hal_feature_config.h"
#ifdef OC_SPI_DEMO_MODULE_ENABLED
#include <stdio.h>
#include "global_types.h"
#include "oc_spi.h"

char spi_test_rx_buffer[64];
char spi_test_tx_buffer[]="1,2,3,4,5,6,7,8,9,a,b,c,d,e,f";
void customer_app_spi_demo(void)
{
	UINT16 dataLength = sizeof(spi_test_tx_buffer);
	
	OC_UART_LOG_Printf("-----customer_app_spi_demo--------\n");

	OC_SPI_Init(OC_SPI_MODE0, OC_SPI_CLK_13MHZ);
	
	//spi test:miso connet mosi 
	OC_SPI_DataTransfer(spi_test_rx_buffer, spi_test_tx_buffer, dataLength);

	OC_UART_LOG_Printf("SPI_DataTransfer,TX:[%s], RX=[%s] \n",spi_test_tx_buffer, spi_test_rx_buffer);

	OC_SPI_WriteData(spi_test_tx_buffer, dataLength);
	OC_SPI_ReadData(spi_test_rx_buffer,dataLength);
	OC_UART_LOG_Printf("SPI_WriteData TX:[%s], SPI_ReadData RX=[%s] \n",spi_test_tx_buffer, spi_test_rx_buffer);

}
#endif
