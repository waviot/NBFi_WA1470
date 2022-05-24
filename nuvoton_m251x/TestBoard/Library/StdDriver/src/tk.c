/**************************************************************************//**
 * @file     tk.c
 * @version  V3.00
 * @brief    Touch key driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "NuMicro.h"

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup TK_Driver TK Driver
  @{
*/


/** @addtogroup TK_EXPORTED_FUNCTIONS TK Exported Functions
  @{
*/


/**
 * @brief Enable touch key function
 * @param None
 * @return None
 * @note This function will enable touch key function and initial idle and polarity state as GND first for all scan keys
 * \hideinitializer
 */

void TK_Open(void)
{
    TK->SCANC |= TK_SCANC_TK_EN_Msk;

    /* Set idle and polarity state as GND */
    TK->IDLSC = 0;
    TK->POLSEL = 0;

    if ((SYS->PDID & 0x01925000) == 0x01925000)
    {
        //for M258G
        TK->IDLSC1 = 0;
        TK->POLSEL1 = 0;
    }

    TK->POLC &= ~(TK_POLC_IDLS16_Msk | TK_POLC_POL16_Msk);
}

/**
 * @brief Disable touch key function
 * @param None
 * @return None
 * \hideinitializer
 */
void TK_Close(void)
{
    TK->SCANC &= ~TK_SCANC_TK_EN_Msk;
}

/**
 * @brief Set touch key scan mode
 * @param[in] u32Mode Single ,periodic or all key scan mode
 *              - \ref TK_SCAN_MODE_SINGLE
 *              - \ref TK_SCAN_MODE_PERIODIC
 *              - \ref TK_SCAN_MODE_ALL_KEY
 *              - \ref TK_SCAN_MODE_PERIODIC_ALL_KEY
 * @return None
 * @details This function is used to set touch key scan mode.
 * @note If touch key controller sets as periodic mode, touch key will be trigger scan by Timer0. So Timer0 must be enabled and operated in periodic mode.
 *       If touch key controller sets as single scan mode, touch key can be trigger scan by calling TK_START_SCAN().
 * \hideinitializer
 */
void TK_SetScanMode(uint32_t u32Mode)
{
    TK->SCANC &= ~TK_SCANC_TMRTRG_EN_Msk;
    TK->REFC &= ~TK_REFC_SCAN_ALL_Msk;

    if (u32Mode == TK_SCAN_MODE_PERIODIC)
    {
        TK->SCANC |= u32Mode;
    }
    else if (u32Mode == TK_SCAN_MODE_ALL_KEY)
    {
        TK->REFC |= u32Mode;
    }
    else if (u32Mode == TK_SCAN_MODE_PERIODIC_ALL_KEY)
    {
        TK->SCANC |= TK_SCANC_TMRTRG_EN_Msk;
        TK->REFC |= TK_REFC_SCAN_ALL_Msk;
    }
}

/**
 * @brief Configure touch key scan sensitivity
 * @param[in] u32PulseWidth Sensing pulse width
 *              - \ref TK_SENSE_PULSE_1
 *              - \ref TK_SENSE_PULSE_2
 *              - \ref TK_SENSE_PULSE_4
 *              - \ref TK_SENSE_PULSE_8
 *              - \ref TK_SENSE_PULSE_250NS
 *              - \ref TK_SENSE_PULSE_500NS
 * @param[in] u32SenseCnt Sensing count
 *              - \ref TK_SENSE_CNT_128
 *              - \ref TK_SENSE_CNT_255
 *              - \ref TK_SENSE_CNT_511
 *              - \ref TK_SENSE_CNT_1023
 *              - \ref TK_SENSE_CNT_8
 *              - \ref TK_SENSE_CNT_16
 *              - \ref TK_SENSE_CNT_32
 *              - \ref TK_SENSE_CNT_64
 * @param[in] u32AVCCHSel voltage selection
 *              - \ref TK_AVCCH_1_DIV_16
 *              - \ref TK_AVCCH_1_DIV_8
 *              - \ref TK_AVCCH_3_DIV_16
 *              - \ref TK_AVCCH_1_DIV_4
 *              - \ref TK_AVCCH_5_DIV_16
 *              - \ref TK_AVCCH_3_DIV_8
 *              - \ref TK_AVCCH_7_DIV_16
 *              - \ref TK_AVCCH_1_DIV_2
 * @return None
 * @details This function is used to configure touch key scan sensitivity.
 * \hideinitializer
 */
void TK_ConfigSensitivity(uint32_t u32PulseWidth, uint32_t u32SenseCnt, uint32_t u32AVCCHSel)
{
    TK->REFC = (TK->REFC & ~(TK_REFC_SENSET_Msk | TK_REFC_PULSET_Msk)) | (u32PulseWidth | u32SenseCnt);
    TK_SET_AVCCH(u32AVCCHSel);
}

/**
 * @brief Set touch key capacitor bank polarity
 * @param[in] u32CapBankPolSel capacitor bank polarity selection
 *              - \ref TK_CAP_BANK_POL_SEL_GND
 *              - \ref TK_CAP_BANK_POL_SEL_AVCCH
 *              - \ref TK_CAP_BANK_POL_SEL_VDD
 * @return None
 * @details This function is used to set touch key capacitor bank polarity.
 * \hideinitializer
 */
void TK_SetCapBankPol(uint32_t u32CapBankPolSel)
{
    TK->POLC = (TK->POLC & ~TK_POLC_POL_CAP_Msk) | (u32CapBankPolSel << TK_POLC_POL_CAP_Pos);
}

/**
 * @brief Configure touch key polarity
 * @param[in] u32Mask Combination of touch keys which need to be configured
 * @param[in] u32PolSel touch key polarity selection
 *              - \ref TK_TKn_POL_SEL_GND
 *              - \ref TK_TKn_POL_SEL_AVCCH
 *              - \ref TK_TKn_POL_SEL_VDD
 * @return None
 * @details This function is used to configure touch key polarity.
 * \hideinitializer
 */
void TK_SetTkPol(uint32_t u32Mask, uint32_t u32PolSel)
{
    uint32_t i;

    if ((1ul << 16) & u32Mask)
        TK->POLC = (TK->POLC & ~TK_POLC_POL16_Msk) | (u32PolSel << TK_POLC_POL16_Pos);

    for (i = 0 ; i < 16 ; i++)
    {
        if ((1ul << i) & u32Mask)
            TK->POLSEL = (TK->POLSEL & ~(TK_POLSEL_POL0_Msk << (i * 2))) | (u32PolSel << (i * 2));
    }

    for (i = 17 ; i < 26 ; i++)
    {
        if ((1ul << i) & u32Mask)
            TK->POLSEL1 = (TK->POLSEL1 & ~(TK_POLSEL_POL0_Msk << ((i - 17) * 2))) | (u32PolSel << ((i - 17) * 2));
    }
}

/**
 * @brief Enable the polarity of specified touch key(s)
 * @param[in] u32Mask Combination of enabled scan keys. Each bit corresponds to a touch key
 *                           Bit 0 represents touch key 0, bit 1 represents touch key 1...
 * @return None
 * @details This function is used to enable the polarity of specified touch key(s).
 * \hideinitializer
 */
void TK_EnableTkPolarity(uint32_t u32Mask)
{
    TK->POLC |= ((u32Mask & 0x1FFFF) << TK_POLC_POLEN0_Pos);

    if ((SYS->PDID & 0x01925000) == 0x01925000)
    {
        TK->POLC1 |= (u32Mask >> 17);
    }
}

/**
 * @brief Disable the polarity of specified touch key(s)
 * @param[in] u32Mask Combination of enabled scan keys. Each bit corresponds to a touch key
 *                           Bit 0 represents touch key 0, bit 1 represents touch key 1...
 * @return None
 * @details This function is used to disable the polarity of specified touch key(s).
 * \hideinitializer
 */
void TK_DisableTkPolarity(uint32_t u32Mask)
{
    TK->POLC &= ~((u32Mask & 0x1FFFF) << TK_POLC_POLEN0_Pos);

    if ((SYS->PDID & 0x01925000) == 0x01925000)
    {
        TK->POLC1 &= ~(u32Mask >> 17);
    }
}

/**
 * @brief Set complement capacitor bank data of specified touch key
 * @param[in] u32TKNum Touch key number. The valid value is 0~25.
 * @param[in] u32CapData Complement capacitor bank data. The valid value is 0~0xFF.
 * @return None
 * @details This function is used to set complement capacitor bank data of specified touch key.
 * \hideinitializer
 */
void TK_SetCompCapBankData(uint32_t u32TKNum, uint32_t u32CapData)
{
    if (u32TKNum <= 16)
    {
        *(__IO uint32_t *)(&(TK->CCBD0) + ((u32TKNum % 17) >> 2)) &= ~(TK_CCBD0_CCBD0_Msk << ((u32TKNum % 17) % 4 * 8));
        *(__IO uint32_t *)(&(TK->CCBD0) + ((u32TKNum % 17) >> 2)) |= (u32CapData << ((u32TKNum % 17) % 4 * 8));
    }
    else
    {
        *(__IO uint32_t *)(&(TK->CCBD5) + ((u32TKNum % 17) >> 2)) &= ~(TK_CCBD0_CCBD0_Msk << ((u32TKNum % 17) % 4 * 8));
        *(__IO uint32_t *)(&(TK->CCBD5) + ((u32TKNum % 17) >> 2)) |= (u32CapData << ((u32TKNum % 17) % 4 * 8));
    }
}

/**
 * @brief Set complement capacitor bank data of reference touch key
 * @param[in] u32CapData Complement capacitor bank data. The valid value is 0~0xFF.
 * @return None
 * @details This function is used to set complement capacitor bank data of reference touch key.
 * \hideinitializer
 */
void TK_SetRefKeyCapBankData(uint32_t u32CapData)
{
    /* In M258, each channel has own reference capacitor data. The function will be used if SCAN_ALL */
    TK->CCBD4 = (TK->CCBD4 & ~TK_CCBD4_CCBD_ALL_Msk) | (u32CapData << TK_CCBD4_CCBD_ALL_Pos);
}

/**
  * @brief      Set reference capacitor bank data of specified touch key
  * @param[in]  u32TKNum: Touch key number. The valid value is 0~25.
  * @param[in]  u32CapData: Complement capacitor bank data. The valid value is 0~0xFF.
  * @return     None
  * @details    This function is used to set complement capacitor bank data of reference touch key.
  */

void TK_SetRefCapBankData(uint32_t u32TKNum, uint32_t u32CapData)
{
    if (u32TKNum <= 16)
    {
        *(__IO uint32_t *)(&(TK->TK_REFCBD0) + ((u32TKNum % 17) >> 2)) &= ~(TK_REFCBD0_CBD0_Msk << ((u32TKNum % 17) % 4 * 8));
        *(__IO uint32_t *)(&(TK->TK_REFCBD0) + ((u32TKNum % 17) >> 2)) |= (u32CapData << ((u32TKNum % 17) % 4 * 8));
    }
    else
    {
        *(__IO uint32_t *)(&(TK->TK_REFCBD5) + ((u32TKNum % 17) >> 2)) &= ~(TK_REFCBD0_CBD0_Msk << ((u32TKNum % 17) % 4 * 8));
        *(__IO uint32_t *)(&(TK->TK_REFCBD5) + ((u32TKNum % 17) >> 2)) |= (u32CapData << ((u32TKNum % 17) % 4 * 8));
    }
}

/**
 * @brief Set high and low threshold of specified touch key for threshold control interrupt
 * @param[in] u32TKNum Touch key number. The valid value is 0~25.
 * @param[in] u32HighLevel High level for touch key threshold control. The valid value is 0~0xFF.
 * @return None
 * @details This function is used to set high and low threshold of specified touch key for threshold control interrupt.
 * \hideinitializer
 */
void TK_SetScanThreshold(uint32_t u32TKNum, uint32_t u32HighLevel)
{
    if (u32TKNum <= 16)
    {
        *(__IO uint32_t *)(&(TK->THC01) + ((u32TKNum % 17) >> 1)) &= ~((TK_THC01_HTH0_Msk) << (((u32TKNum % 17) & 0x1) * 16));
        *(__IO uint32_t *)(&(TK->THC01) + ((u32TKNum % 17) >> 1)) |= (u32HighLevel << (TK_THC01_HTH0_Pos + ((u32TKNum % 17) & 0x1) * 16));
    }
    else
    {
        *(__IO uint32_t *)(&(TK->THC1718) + ((u32TKNum % 17) >> 1)) &= ~((TK_THC01_HTH0_Msk) << (((u32TKNum % 17) & 0x1) * 16));
        *(__IO uint32_t *)(&(TK->THC1718) + ((u32TKNum % 17) >> 1)) |= (u32HighLevel << (TK_THC01_HTH0_Pos + ((u32TKNum % 17) & 0x1) * 16));
    }
}

/**
 * @brief Enable touch key scan interrupt
 * @param[in] u32Msk Interrupt type selection.
 *              - \ref TK_INT_EN_SCAN_COMPLETE
 *              - \ref TK_INT_EN_SCAN_COMPLETE_LEVEL_TH
 * @return None
 * @details This function is used to enable touch key scan interrupt.
 * @note It need disable the enabled interrupt type first by TK_DisableInt() before to change enabled interrupt type.
 * \hideinitializer
 */
void TK_EnableInt(uint32_t u32Msk)
{
    TK->INTEN |= u32Msk;
}

/**
 * @brief Disable touch key scan interrupt
 * @param[in] u32Msk Interrupt type selection.
 *              - \ref TK_INT_EN_SCAN_COMPLETE
 *              - \ref TK_INT_EN_SCAN_COMPLETE_LEVEL_TH
 * @return None
 * @details This function is used to disable touch key scan interrupt.
* @note It need disable the enabled interrupt type first by TK_DisableInt() before to change enabled interrupt type.
 * \hideinitializer
 */
void TK_DisableInt(uint32_t u32Msk)
{
    TK->INTEN &= ~u32Msk;
}

/**
  * @brief      To disable all channels
  * @param[in]  None
  * @return     None
  * @details    This function is used to disable all channels for key scan.
  */
void TK_DisableAllChannel(void)
{
    TK->SCANC &= ~(0x1FFFF);

    if ((SYS->PDID & 0x01925000) == 0x01925000)
    {
        TK->SCANC1 &= ~(0x1F);
    }
}


/**
  * @brief      To clear all interrupts that were caused if the conversion TK data over the high threshold.
  * @param[in]  None
  * @return     None
  * @details    This function is used to clear all interrupts that were caused if the conversion TK data over the high threshold.
  */
void TK_ClearTKIF(void)
{
    TK->STA |= 0x1FFFFC3UL;

    if ((SYS->PDID & 0x01925000) == 0x01925000)
    {
        TK->STA1 |= 0x1FUL;
    }
}

/**
  * @brief      To enable scan all function to wake up system by any touch keys as low power mode
  * @param[in]  u8RefcbAll: The value co-works with u8CcbAll to make conversion all enabled touch keys' data - TKDATALL close to 0.
  * @param[in]  u8CcbAll: The value co-works with u8RefcbAll to make conversion all enabled touch keys' data - TKDATALL close to 0.
  * @param[in]  u8HThAll: Threshold to wake up system by any touch keys as low power mode
  * @return     None
  * @details    The u8RefcbAll and u8CcbAll are the calibration values was generate by calibration flow
  *             The flow makes the TKDATALL close to 0 after scan all enabled all touch keys.
  */
void TK_EnableScanAll(uint8_t u8RefcbAll, uint8_t u8CcbAll, uint8_t u8HThAll)
{
    TK->REFC |= TK_REFC_SCAN_ALL_Msk;
    TK->TK_REFCBD4 = (TK->TK_REFCBD4 & (~TK_REFCBD4_CBD_ALL_Msk)) | (u8RefcbAll << TK_REFCBD4_CBD_ALL_Pos);
    TK->CCBD4 = (TK->CCBD4 & (~TK_CCBD4_CCBD_ALL_Msk)) | (u8CcbAll << TK_CCBD4_CCBD_ALL_Pos);
    TK->THC16 = (TK->THC16 & (~TK_THC16_HTH_ALL_Msk))  | (u8HThAll << TK_THC16_HTH_ALL_Pos);
}

/**
  * @brief      To disable scan all function to wake up system by any touch keys as low power mode
  * @param[in]  None
  * @return     None
  */
void TK_DisableScanAll(void)
{
    TK->REFC &= ~TK_REFC_SCAN_ALL_Msk;
}

/*@}*/ /* end of group TK_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group TK_Driver */

/*@}*/ /* end of group Standard_Driver */
