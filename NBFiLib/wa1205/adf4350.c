
//#include <malloc.h>
#include <stdio.h>
#include "adf4350.h"


void init_config(adf4350_state *st,
                 adf4350_platform_data *pdata)
{
    pdata->ref_doubler_en = 0;
    pdata->ref_div2_en = 0;
    pdata->ref_div_factor = 2;
    pdata->r2_user_settings = 0x8E42;
    pdata->r3_user_settings = 0x4B3;
    pdata->r4_user_settings = 0xAA003C;////0xAA0024;//0xA200A4;//0xAA003C;
    st->pdata = pdata;

    st->clkin = 40000000; // 40 MHz
    st->chspc = 1000; // 100 KHz
    st->fpfd = 20000000; // 20 MHz
}

/***************************************************************************//**
 * @brief Increases the R counter value until the ADF4350_MAX_FREQ_PFD is
 *        greater than PFD frequency.
 *
 * @param st    - The selected structure.
 * @param r_cnt - Initial r_cnt value.
 *
 * @return Returns 0 in case of success or negative error code.
*******************************************************************************/
int32_t adf4350_tune_r_cnt(adf4350_state *st, uint16_t r_cnt)
{
    adf4350_platform_data *pdata = st->pdata;

	do
	{
		r_cnt++;
		st->fpfd = (st->clkin * (pdata->ref_doubler_en ? 2 : 1)) /
			   (r_cnt * (pdata->ref_div2_en ? 2 : 1));
	} while (st->fpfd > ADF4350_MAX_FREQ_PFD);

	return r_cnt;
}

/***************************************************************************//**
 * @brief Computes the greatest common divider of two numbers
 *
 * @return Returns the gcd.
*******************************************************************************/
uint32_t gcd(uint32_t x, uint32_t y)
{
	int32_t tmp;

	tmp = y > x ? x : y;


	while((x % tmp) || (y % tmp))
	{
		tmp--;
	}

	return tmp;
}

/***************************************************************************//**
 * @brief Sets the ADF4350 frequency.
 *
 * @param st   - The selected structure.
 * @param freq - The desired frequency value.
 *
 * @return calculatedFrequency - The actual frequency value that was set.
*******************************************************************************/
int64_t adf4350_set_freq(adf4350_state *st, uint64_t freq) {
    adf4350_platform_data *pdata = st->pdata;
    uint64_t tmp;
    uint32_t div_gcd, prescaler, chspc;
    uint16_t mdiv, r_cnt = 0;
    uint8_t band_sel_div;
    int32_t ret;

    if ((freq > ADF4350_MAX_OUT_FREQ) || (freq < ADF4350_MIN_OUT_FREQ))
        return -1;

    if (freq > ADF4350_MAX_FREQ_45_PRESC) {
        prescaler = ADF4350_REG1_PRESCALER;
        mdiv = 75;
    } else {
        prescaler = 0;
        mdiv = 23;
    }

    st->r4_rf_div_sel = 0;

    while (freq < ADF4350_MIN_VCO_FREQ) {
        freq <<= 1;
        st->r4_rf_div_sel++;
    }

    /*
     * Allow a predefined reference division factor
     * if not set, compute our own
     */
    if (pdata->ref_div_factor)
        r_cnt = pdata->ref_div_factor - 1;

    chspc = st->chspc;

    do {
        do {
            do {
                r_cnt = adf4350_tune_r_cnt(st, r_cnt);
                st->r1_mod = st->fpfd / chspc;
                if (r_cnt > ADF4350_MAX_R_CNT) {
                    /* try higher spacing values */
                    chspc++;
                    r_cnt = 0;
                }
            } while ((st->r1_mod > ADF4350_MAX_MODULUS) && r_cnt);
        } while (r_cnt == 0);


        tmp = freq * (uint64_t) st->r1_mod + (st->fpfd > 1);

        tmp = (tmp / st->fpfd);    /* Div round closest (n + d/2)/d */

        st->r0_fract = tmp % st->r1_mod;
        tmp = tmp / st->r1_mod;

        st->r0_int = tmp;
    } while (mdiv > st->r0_int);

    band_sel_div = st->fpfd % ADF4350_MAX_BANDSEL_CLK > ADF4350_MAX_BANDSEL_CLK / 2 ?
                   st->fpfd / ADF4350_MAX_BANDSEL_CLK + 1 :
                   st->fpfd / ADF4350_MAX_BANDSEL_CLK;

    if (st->r0_fract && st->r1_mod) {
        div_gcd = gcd(st->r1_mod, st->r0_fract);
        st->r1_mod /= div_gcd;
        st->r0_fract /= div_gcd;
    } else {
        st->r0_fract = 0;
        st->r1_mod = 1;
    }

    st->regs[ADF4350_REG0] = ADF4350_REG0_INT(st->r0_int) |
                             ADF4350_REG0_FRACT(st->r0_fract);

    st->regs[ADF4350_REG1] = 1 | ADF4350_REG1_PHASE(1) |
                             ADF4350_REG1_MOD(st->r1_mod) |
                             prescaler;

    st->regs[ADF4350_REG2] = 2 |
            ADF4350_REG2_10BIT_R_CNT(r_cnt) |
            (pdata->ref_doubler_en ? ADF4350_REG2_RMULT2_EN : 0) |
            (pdata->ref_div2_en ? ADF4350_REG2_RDIV2_EN : 0) |
            (pdata->r2_user_settings & (ADF4350_REG2_PD_POLARITY_POS |
                                        ADF4350_REG2_LDP_10ns | ADF4350_REG2_LDF_FRACT_N |
                                        ADF4350_REG2_CHARGE_PUMP_CURR_uA(2500)));

    st->regs[ADF4350_REG3] = 3 | pdata->r3_user_settings;

    st->regs[ADF4350_REG4] = 4 |
            ADF4350_REG4_FEEDBACK_FUND |
            ADF4350_REG4_RF_DIV_SEL(st->r4_rf_div_sel) |
            ADF4350_REG4_8BIT_BAND_SEL_CLKDIV(band_sel_div) |
            ADF4350_REG4_RF_OUT_EN |
            (pdata->r4_user_settings &
             (ADF4350_REG4_OUTPUT_PWR(0x03) |
              ADF4350_REG4_AUX_OUTPUT_PWR(0x3) |
              ADF4350_REG4_AUX_OUTPUT_EN |
              ADF4350_REG4_AUX_OUTPUT_DIV));

    st->regs[ADF4350_REG5] = 5 | ADF4350_REG5_LD_PIN_MODE_DIGITAL | 0x00180000;

    if (ret < 0) {
        return ret;
    }

    tmp = (uint64_t) ((st->r0_int * st->r1_mod) + st->r0_fract) * (uint64_t) st->fpfd;
    tmp = tmp / ((uint64_t) st->r1_mod * ((uint64_t) 1 << st->r4_rf_div_sel));

    return tmp;
}


