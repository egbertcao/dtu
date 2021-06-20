/******************************************************************************
 *
 *  (C)Copyright ASRMicro. All Rights Reserved.
 *
 *  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF ASRMicro.
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *  This Module contains Proprietary Information of ASRMicro and should be
 *  treated as Confidential.
 *  The information in this file is provided for the exclusive use of the
 *  licensees of ASRMicro.
 *  Such users have the right to use, modify, and incorporate this code into
 *  products for purposes authorized by the license agreement provided they
 *  include this notice and the associated copyright notice with any such
 *  product.
 *  The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

/******************************************************************************
*               MODULE IMPLEMENTATION FILE
*******************************************************************************
* Title: codec nau8814 related
*
* Filename: acm_nau8814.c
*
* Target, platform: Common Platform, SW platform
*
* Authors: Chao Liu
*
* Description: Controls nau8814.
*
* Last Updated:
*
* Notes:
*******************************************************************************/
#include "oc_i2c.h"
#include "oc_gpio.h"

#ifdef OC_I2C_DEMO_MODULE_ENABLED

#define NAU8814_I2C_SLAVE_ADDR               0x1A    //7-MSB bits: 0011_010
#define NAU8814_SLAVE_WRITE_ADDR             0x34
#define NAU8814_SLAVE_READ_ADDR              0x35
    
static UINT8 l_connect_nau8814 = 0;
OC_GPIORC_PIN_NUM i2c_scl_pin = OC_GPIORC_PIN_SCL;
OC_GPIORC_PIN_NUM i2c_sda_pin = OC_GPIORC_PIN_SDA;
OC_GPIORC_PINMUX_FUNCTION i2c_scl_pin_mode = OC_GPIORC_PIN_SCL_M_CI2C_SCL;
OC_GPIORC_PINMUX_FUNCTION i2c_sda_pin_mode = OC_GPIORC_PIN_SDA_M_CI2C_SDA;

OC_GPIORC_PIN_NUM pcm_out_pin = OC_GPIORC_PIN_PCM_OUT;
OC_GPIORC_PIN_NUM pcm_in_pin = OC_GPIORC_PIN_PCM_IN;
OC_GPIORC_PIN_NUM pcm_sync_pin = OC_GPIORC_PIN_PCM_SYNC;
OC_GPIORC_PIN_NUM pcm_clk_pin = OC_GPIORC_PIN_PCM_CLK;
OC_GPIORC_PINMUX_FUNCTION pcm_out_pin_mode = OC_GPIORC_PIN_PCM_OUT_M_SSPA_DATA_OUT;
OC_GPIORC_PINMUX_FUNCTION pcm_in_pin_mode = OC_GPIORC_PIN_PCM_IN_M_SSPA_DATA_IN;
OC_GPIORC_PINMUX_FUNCTION pcm_sync_pin_mode = OC_GPIORC_PIN_PCM_SYNC_M_SSPA_SYNC;
OC_GPIORC_PINMUX_FUNCTION pcm_clk_pin_mode = OC_GPIORC_PIN_PCM_CLK_M_SSPA_BITCLK;
OC_GPIORC_PIN_NUM nau8814_power_en_pin = OC_GPIORC_PIN_WAKEUP_OUT;
OC_GPIORC_PINMUX_FUNCTION nau8814_power_en_pin_mode = OC_GPIORC_PIN_WAKEUP_OUT_M_GPIO;

/******************************************************************************
* Function     : nau8814_reg_read
*******************************************************************************
*
* Description  :  
*
* Parameters   : UINT8 RegAddr
* Parameters   : UINT16 *value
*
* Output Param   : None.
*
* Return value   :  
*
* Notes		  : 

|-----|----------------------|-----|-----------------|---|-----|-----|---------------------|...
|     |<- slave    address ->|     |<- reg address ->|NA |     |     | <-slave   address-> |...
|-----|----------------------|-----|-----------------|---|-----|-----|---------------------|...
|start|0|0|1|1|0|1|0|WRITE(0)| ACK |A6|A5|A4|A3|A2|A1| 0 | ACK |start|0|0|1|1|0|1|0|READ(1)|...
|----------------------------|-----|---------------------|-----|---------------------------|...
|        master send         |slave|     master send     |slave|     master  send          |...
|----------------------------|-----|---------------------|-----|---------------------------|...

... |----------------|------|-----------------------|-----------|
... |<- high data  ->|      |<-     low data      ->|           |
... |----------------|------|-----------------------|-----------|
... |0|0|0|0|0|0|0|D8| ACK  |D7|D6|D5|D4|D3|D2|D1|D0| NACK |stop|
... |----------------|------|-----------------------|-----------|
... |  slave  send   |master|      slave  send      |  master   |
... |----------------|------|-----------------------|-----------|

*******************************************************************************/
OC_I2C_ReturnCode oc_nau8814_reg_read(UINT8 RegAddr, UINT16 *value)
{
	int ret;
	UINT8 nau_reg = (RegAddr << 1) & 0xFE;

	ret = OC_REG_BYTE_READ_SHORT(NAU8814_I2C_SLAVE_ADDR, nau_reg);	
	if (ret != -1){
		*value = ret;
		return OC_I2C_RC_OK;
	}
	return OC_I2C_RC_NOT_OK;
}

/******************************************************************************
* Function     : nau8814_reg_write
*******************************************************************************
*
* Description  :  
*
* Parameters   : UINT8 RegAddr
* Parameters   : UINT16 RegData
*
* Output Param   : None.
*
* Return value   :  
*
* Notes		  : 

|-----|----------------------|-----|---------------------|-----|-----------------------|-----|------|
|     |<- slave    address ->|     |<- reg address ->| D8|     |<-     low  data     ->|     |      |
|-----|----------------------|-----|---------------------|-----|-----------------------|-----|------|
|start|0|0|1|1|0|1|0|WRITE(0)| ACK |A6|A5|A4|A3|A2|A1| D8| ACK |D7|D6|D5|D4|D3|D2|D1|D0| ACK | stop |
|----------------------------|-----|---------------------|-----|-----------------------|-----|------|
|        master send         |slave|     master send     |slave|     master  send      |slave|master|
|----------------------------|-----|---------------------|-----|-----------------------|-----|------|

*******************************************************************************/
OC_I2C_ReturnCode oc_nau8814_reg_write(UINT8 RegAddr, UINT16 RegData)
{
	int ret;
	UINT8 param_data[3] = {0x00};
	param_data[0] = ((RegAddr<<1) & 0xFE) | ((RegData >> 8) & 0x01);
	param_data[1] = (UINT8)(RegData & 0xFF);
	
 	ret = OC_REG_BYTE_WRITE_SHORT(NAU8814_I2C_SLAVE_ADDR, param_data[0], param_data[1]);
	
	if (ret == 0){
		return OC_I2C_RC_OK;
	}
	return OC_I2C_RC_NOT_OK;
}
/******************************************************************************
* Function     : codec_nau8814_enable_mfpr
*******************************************************************************
*
* Description  :  set GPIO-10 and GPIO-11 as function of I2C, enable I2C clock
*
* Parameters   : 
*
* Output Param   : None.
*
* Return value   :  
*
* Notes		  : 
*******************************************************************************/
void codec_nau8814_enable(void)
{
	OC_UART_LOG_Printf("codec_nau8814_enable_mfpr\r\n");	
	//I2C
	OC_GpioSetFunction(i2c_scl_pin, i2c_scl_pin_mode);
	OC_GpioSetFunction(i2c_sda_pin, i2c_sda_pin_mode);

	//PCM 
	OC_GpioSetFunction(pcm_out_pin, pcm_out_pin_mode);
	OC_GpioSetFunction(pcm_in_pin, pcm_in_pin_mode);
	OC_GpioSetFunction(pcm_sync_pin, pcm_sync_pin_mode);
	OC_GpioSetFunction(pcm_clk_pin, pcm_clk_pin_mode);

	//CRYSTAL VCC_EN
	OC_GpioSetFunction(nau8814_power_en_pin, nau8814_power_en_pin_mode);
	OC_GpioSetDirection(nau8814_power_en_pin, OC_GPIO_DIRECTION_OUTPUT);
	OC_GpioSetLevel(nau8814_power_en_pin, OC_GPIORC_HIGH);
	return;
}

int codec_nau8814_check_device_id(void)
{    
	static UINT8 nau8814_init_ok = 0;
	UINT16 value = 0;

	if(0 == nau8814_init_ok){
		codec_nau8814_enable();
		OC_I2C_Init(OC_STANDARD_MODE);
		/* check if has codec nau8814*/
		if((OC_I2C_RC_OK == oc_nau8814_reg_read(0x3F, &value)) 
			&& (NAU8814_I2C_SLAVE_ADDR == value )){
			OC_UART_LOG_Printf("codec_nau8814_check_device_id, REG0x3F=%x !\r\n", value);	
			oc_nau8814_reg_write(0, 0x00);    
			nau8814_init_ok = 1;
		}

		OC_UART_LOG_Printf("file:%s,function:%s,line:%d, nau8814_init_ok=%d !\r\n", __FILE__,__func__,__LINE__, nau8814_init_ok);	

		oc_nau8814_reg_read(0x40, &value);//REG40=0xCA
		OC_UART_LOG_Printf("codec_nau8814_check_device, REG0x40=%x !\r\n", value);	
		oc_nau8814_reg_read(0x41, &value);//REG41=0x124
		OC_UART_LOG_Printf("codec_nau8814_check_device, REG0x41=%x !\r\n", value);	
	}

	return nau8814_init_ok;
}
#endif
