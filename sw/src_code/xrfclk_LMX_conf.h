
/*****************************************************************************/
/**
*
* @file xrfclk_LMX_conf.h
* @addtogroup xrfclk_LMX_conf_v1_1
* @{
*
* Contains the configuration data for LMX.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     08/28/19 Initial version
*
* </pre>
*
******************************************************************************/
#ifndef __XRFCLK_LMX_CONF_H_
#define __XRFCLK_LMX_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xrfclk.h"

// Use TICS pro to see configuration

/* Frequency configurations for LMX2594(A/B) PLL */
const u32 LMX2594[LMX_ADC_NUM][LMX2594_COUNT] = {
		// 491.52 MHz
		{
				0x700000,
			    0x6F0000,
			    0x6E0000,
			    0x6D0000,
			    0x6C0000,
			    0x6B0000,
			    0x6A0000,
			    0x690021,
			    0x680000,
			    0x670000,
			    0x663F80,
			    0x650011,
			    0x640000,
			    0x630000,
			    0x620200,
			    0x610888,
			    0x600000,
			    0x5F0000,
			    0x5E0000,
			    0x5D0000,
			    0x5C0000,
			    0x5B0000,
			    0x5A0000,
			    0x590000,
			    0x580000,
			    0x570000,
			    0x560000,
			    0x55D300,
			    0x540001,
			    0x530000,
			    0x521E00,
			    0x510000,
			    0x506666,
			    0x4F0026,
			    0x4E00E5,
			    0x4D0000,
			    0x4C000C,
			    0x4B0940,
			    0x4A0000,
			    0x49003F,
			    0x480001,
			    0x470081,
			    0x46C350,
			    0x450000,
			    0x4403E8,
			    0x430000,
			    0x4201F4,
			    0x410000,
			    0x401388,
			    0x3F0000,
			    0x3E0322,
			    0x3D00A8,
			    0x3C0000,
			    0x3B0001,
			    0x3A8001,
			    0x390020,
			    0x380000,
			    0x370000,
			    0x360000,
			    0x350000,
			    0x340820,
			    0x330080,
			    0x320000,
			    0x314180,
			    0x300300,
			    0x2F0300,
			    0x2E07FC,
			    0x2DC0DF,
			    0x2C1F20,
			    0x2B0000,
			    0x2A0000,
			    0x290000,
			    0x280000,
			    0x270001,
			    0x260000,
			    0x250104,
			    0x240140,
			    0x230004,
			    0x220000,
			    0x211E21,
			    0x200393,
			    0x1F43EC,
			    0x1E318C,
			    0x1D318C,
			    0x1C0488,
			    0x1B0002,
			    0x1A0DB0,
			    0x190624,
			    0x18071A,
			    0x17007C,
			    0x160001,
			    0x150401,
			    0x14C848,
			    0x1327B7,
			    0x120064,
			    0x110117,
			    0x100080,
			    0x0F064F,
			    0x0E1E40,
			    0x0D4000,
			    0x0C5001,
			    0x0B00A8,
			    0x0A10D8,
			    0x090604,
			    0x082000,
			    0x0740B2,
			    0x06C802,
			    0x0500C8,
			    0x040C43,
			    0x030642,
			    0x020500,
			    0x010809,
			    0x00241C,

	}

};

#ifdef __cplusplus
}
#endif

#endif
/** @} */
