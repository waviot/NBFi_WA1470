/**
  ******************************************************************************
  * File Name          : CRC.c
  * Description        : This file provides code for the configuration
  *                      of the CRC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "crc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* CRC init function */
void MX_CRC_Init(void)
{

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);

  LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
  LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
  LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE);

}

/* USER CODE BEGIN 1 */
uint32_t CRC_Crc32(uint32_t aDataBuffer[], uint32_t bufferLength, uint32_t initValue)
{

  register uint32_t data = 0;
  register uint32_t index = 0;
  LL_CRC_ResetCRCCalculationUnit(CRC);
  /* Compute the CRC of Data Buffer array*/
  for (index = 0; index < (bufferLength / 4); index++)
  {
    //data = (uint32_t)((aDataBuffer[index + 3] << 24) | (aDataBuffer[ index + 2] << 16) | (aDataBuffer[index + 1] << 8) | aDataBuffer[ index]);
      data = aDataBuffer[index];
      LL_CRC_FeedData32(CRC, data);
  }

  /* Return computed CRC value */
  return (LL_CRC_ReadData32(CRC));
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
