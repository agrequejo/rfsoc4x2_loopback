/******************************************************************************
*
* Copyright (C) 2017-2019 Xilinx, Inc.  All rights reserved.
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xrfclk.h
* @addtogroup xrfclk_v1_1
* @{
*
* Contains the API of the XRFclk middleware.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     23/04/26
*
* </pre>
*
******************************************************************************/
#ifndef __XRFCLK_H_
#define __XRFCLK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "xil_types.h"

#define RFCLK_LMX2594_1 1 /* I0 on MUX and SS3 on Bridge */
#define RFCLK_LMX2594_2 2 /* I1 on MUX and SS2 on Bridge */
#define RFCLK_LMK 0 /* I2 on MUX and SS1 on Bridge */

#define RFCLK_CHIP_NUM 3
#define LMK_COUNT 136//128//// 136 //128
#define LMK_FREQ_NUM 1 /* Number of LMK freq. configs */
#define LMX_ADC_NUM 1 /* Number of LMX ADC configs */
#define LMX_DAC_NUM 1 /* Number of LMX DAC configs */


#define LMX2594_COUNT 113
#define FREQ_LIST_STR_SIZE 16 /* Frequency string size */



u32 XRFClk_WriteReg(u32 ChipId, u32 Data);
u32 XRFClk_ReadReg(u32 ChipId, u32 *Data, u32 *out);


u32 XRFClk_Init(void);


void XRFClk_Close();
u32 XRFClk_ResetChip(u32 ChipId);
u32 XRFClk_SetConfigOnOneChipFromConfigId(u32 ChipId, u32 ConfigId);
u32 XRFClk_SetConfigOnOneChip(u32 ChipId, u32 *cfgData, u32 len);
u32 XRFClk_GetConfigFromOneChip(u32 ChipId, u32 *cfgData);
u32 XRFClk_SetConfigOnAllChipsFromConfigId(u32 ConfigId_LMK, u32 ConfigId_RF1,
                                u32 ConfigId_RF2);
u32 XRFClk_ControlOutputPortLMK(u32 PortId, u32 state);
u32 XRFClk_ConfigOutputDividerAndMUXOnLMK(u32 PortId, u32 DCLKoutX_DIV,
					  u32 DCLKoutX_MUX, u32 SDCLKoutY_MUX,
					  u32 SYSREF_DIV);

u32 XRFClk_LMK_readback_activation (u32 ChipId);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
