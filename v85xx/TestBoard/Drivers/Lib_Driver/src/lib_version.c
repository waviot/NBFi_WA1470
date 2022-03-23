/**
*******************************************************************************
  * @file    lib_version.c
  * @author  Application Team
  * @version V4.8.0
  * @date    2020-10-26
  * @brief   Version library.
*******************************************************************************/
#include "lib_version.h"

#define Target_DriveVersion  DRIVER_VERSION(4, 8)

/**
  * @brief  Get Target driver's current version.
  * @param  None
  * @retval Version value
  *           Bit[15:8] : Major version
  *           Bit[7:0]  : Minor version 
  */
uint16_t Target_GetDriveVersion(void)
{
  return (Target_DriveVersion);
}

/*********************************** END OF FILE ******************************/
