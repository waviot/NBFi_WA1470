#include "stm32l0xx_hal.h"

#define PCA9454_I2C 		I2C1

#define PCA9454_ADDR            0x70

#define PCA9454_INP_PORT_ADDR   0x00
#define PCA9454_OUT_PORT_ADDR   0x01
#define PCA9454_POLARITY_ADDR   0x02
#define PCA9454_CONF_ADDR       0x03

I2C_HandleTypeDef hi2c1;

uint8_t pca9454_out_reg = 0;



static void PCA9454_write(uint8_t cmd, uint8_t byte)
{
  uint8_t buf[2];
  buf[0] = cmd;
  buf[1] = byte;
  HAL_I2C_Master_Transmit(&hi2c1, PCA9454_ADDR, buf, 2, HAL_MAX_DELAY);
}

/******* read doesn't work******
static uint8_t PCA9454_read(uint8_t cmd)
{
  uint8_t buf[2];
  buf[0] = cmd;
  HAL_I2C_Master_Transmit(&hi2c1, PCA9454_ADDR, buf, 1, HAL_MAX_DELAY);
  HAL_I2C_Master_Receive(&hi2c1, PCA9454_ADDR, buf, 1, HAL_MAX_DELAY);
  return buf[0];
}*/

void PCA9454_init()
{

  hi2c1.Instance = PCA9454_I2C;
  hi2c1.Init.Timing = 0x0010061A;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  PCA9454_write(PCA9454_CONF_ADDR, 0);  //setup all pins as outputs
  
}


void PCA9454_set_out_pin(uint8_t pin)
{
  pca9454_out_reg |= (1 << pin);
  PCA9454_write(PCA9454_OUT_PORT_ADDR, pca9454_out_reg);
}

void PCA9454_reset_out_pin(uint8_t pin)
{
  pca9454_out_reg &= (~(1 << pin));
  PCA9454_write(PCA9454_OUT_PORT_ADDR, pca9454_out_reg);
}
