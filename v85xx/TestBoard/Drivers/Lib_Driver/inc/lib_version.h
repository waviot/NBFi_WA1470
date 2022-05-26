/**
*******************************************************************************
* @file    lib_version.h
* @author  Application Team
* @version V4.8.0
* @date    2020-10-26
* @brief   Version library.
*******************************************************************************/

#ifndef __LIB_VERSION_H
#define __LIB_VERSION_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "target.h"
   
#define DRIVER_VERSION(major,minor) (((major) << 8) | (minor))

/* Exported Functions ------------------------------------------------------- */
   
/**
  * @brief  Get Target driver's current version.
  * @param  None
  * @retval Version value
  */
uint16_t Target_GetDriveVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_VERSION_H */

/*********************************** END OF FILE ******************************/
