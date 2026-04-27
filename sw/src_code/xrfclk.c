
/*****************************************************************************/
/**
*
* @file xrfclk.c
* @addtogroup Overview
*
*
* Author:      Alejandro García-Requejo
* Institution: University of Alcalá (UAH),
* 			   Electronics Department,
* 		       Geintra Research group
*
* Provides the API for the XRFclk middleware to control the LMK and LMX devices
* via SPI. This implementation has been tested with the RFSoC 4x2 board.
* It is derived from xrfclk.c, an API originally provided by Xilinx,
* which uses an I2C connection between the processor and an SPI bridge
* connected to the clock devices. That implementation was intended for
* the ZCU111 and ZCU208 boards.
*
* A key difference between the ZCU111/ZCU208 boards and the RFSoC 4x2 is that
* the latter requires low-level control of the LMK reset and LMK clock select
* inputs via GPIO (MIO7, MIO8, MIO12) (see ReferenceManual_A6_rfsoc42.pdf,
* p. 24; schematic_rfsoc42.pdf, p. 16).
* Additionally, to read back the LMK clock registers, it is necessary to
* write to register 0x16E, setting the SPI readback bit to 1. This disables
* the pin as an output and configures it as an input, which is connected to
* the SPI SDO pin (see schematic_rfsoc42.pdf, p. 16; lmk4828.pdf, p. 79).
*
* See xrfclk.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc   23/04/26  Initial version
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfclk.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xparameters.h"

#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xspips.h"
#include "xgpiops.h"

#include "xrfclk_LMK_conf.h"
#include "xrfclk_LMX_conf.h"

/******************** Constant Definitions **********************************/
#define MIO_LMK_RST 			7
#define MIO_LMK_CLK_IN_SEL0 	8
#define MIO_LMK_CLK_IN_SEL1 	12

#define PRINTF xil_printf /* Print macro for BM */

/* logging macro */
#define LOG(str)                                                               \
	PRINTF("RFCLK error in %s: %s", __func__, str) /* Logging macro */

#define RF_DATA_READ_BIT 0X80 /* Bit which indicates read */
#define LMX_RESET_VAL 0X2 /* Reset value for LMX */
#define LMK_RESET_VAL 0X80 /* Reset value for LMK04828 */

#define LMK_READBACK_ADDR				0x016E3B

/************************** Function Prototypes ******************************/
static int XRFClk_SPIWrData(XSpiPs *spi0, u8 *Val, u8 Len);
static int XRFClk_SPIRrData(XSpiPs *spi0, u8 *tx, u8 *rx, u8 Len);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XSpiPs Spi0;


/****************************************************************************/
/**
*
* This function is used to open and configure GPIO drivers.
*
* @param	None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_InitGPIO(void)
{
	// Init GPIOs
	XGpioPs Gpio;
	XGpioPs_Config *GpioConfigPtr;

	GpioConfigPtr = XGpioPs_LookupConfig(XPAR_PSU_GPIO_0_DEVICE_ID);
	XGpioPs_CfgInitialize(&Gpio, GpioConfigPtr,
				   GpioConfigPtr->BaseAddr);

	// LMK Sel
	XGpioPs_SetDirectionPin(&Gpio, MIO_LMK_RST, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, MIO_LMK_RST, 1);
	XGpioPs_WritePin(&Gpio, MIO_LMK_RST, 1);
	XGpioPs_WritePin(&Gpio, MIO_LMK_RST, 0);

	// LMX 0 Sel
	XGpioPs_SetDirectionPin(&Gpio, MIO_LMK_CLK_IN_SEL0, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, MIO_LMK_CLK_IN_SEL0, 1);
	XGpioPs_WritePin(&Gpio, MIO_LMK_CLK_IN_SEL0, 0);

	// LMX 1 Sel
	XGpioPs_SetDirectionPin(&Gpio, MIO_LMK_CLK_IN_SEL1, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, MIO_LMK_CLK_IN_SEL1, 1);
	XGpioPs_WritePin(&Gpio, MIO_LMK_CLK_IN_SEL1, 0);

	return XST_SUCCESS;
}



/****************************************************************************/
/**
*
* This function is used to open and configure i2c drivers.
*
* @param	None
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_InitSPI(void)
{
	XSpiPs_Config *Confispipi;
	int Status;

	/*SPIPI */
#ifndef SDT
	Confispipi = XSpiPs_LookupConfig(XPAR_XSPIPS_0_DEVICE_ID);
#else
	Confispipi = XSpiPs_LookupConfig(XPAR_XSPIPS_0_DEVICE_ID);
#endif
	if (Confispipi == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpiPs_CfgInitialize(&Spi0, Confispipi,
				      Confispipi->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    // Perform a self-test to check hardware build
	Status = XSpiPs_SelfTest(&Spi0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    //Status = XSpiPs_SetOptions (&Spi0, XSPIPS_MASTER_OPTION | XSPIPS_MANUAL_START_OPTION );
	//Status = XSpiPs_SetOptions (&Spi0, XSPIPS_MASTER_OPTION);
    XSpiPs_SetOptions(&Spi0, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
    if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    Status = XSpiPs_SetClkPrescaler (&Spi0, XSPIPS_CLK_PRESCALE_256);
    //XSpiPs_SetClkPrescaler(&Spi0, XSPIPS_CLK_PRESCALE_32);
    if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    Status = XSpiPs_SetDelays (&Spi0, 0, 100, 0, 100);
    if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* On the RFSoC 4x2 board, the STATUS_LD2 pin, attached to wire LMK_LD2
* is connected to both the "PLL2 locked" LED and the SPI readback"
* See the bottom of the CLOCK II page, page 16 of the schematic:
* schematic_rfsoc42.pdf
* This pin is, by default, configured to output whether PLL2 is locked,
* but the can be reconfigured to 18 different functions,
* including "SPI readback".
* See PLL2_LD_MUX on page 79 of the LMK04828 datasheet at:
* https://www.ti.com/lit/ds/symlink/lmk04828.pdf
* If this function always returns 0xff or 0x00,
* the PLL2_LD pin is not configured for "SPI readback".
* Instead, in this configuration, 0xff means PLL2 locked
* and the PLL2 LED is on, whereas and 0x00 means it's not.
*
*
* This function Puts the PLL2_LD pin into "SPI readback" and "Output (push-pull)"
* mode. A side effect is that the PLL2 LED on the RFSoc 4x2 board will be mostly off
* except for flashes that are too brief to see when you do SPI reads.
*
* @param	Descriptor for the SPI driver.
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_LMK_readback_activation (u32 ChipId)
{

	if (XST_SUCCESS != XRFClk_WriteReg(ChipId, LMK_READBACK_ADDR)) {
		LOG("reset chip");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function is HAL API for SPI read.
*
* @param	File descriptor for the i2c driver.
* @param	Addr address to be read.
* @param	Val read value.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static int XRFClk_SPIRrData(XSpiPs *spi0, u8 *tx, u8 *rx, u8 Len)
{

	int status =  XSpiPs_PolledTransfer (&Spi0, tx, rx, Len);

	if (status == XST_FAILURE) {
		LOG("read reg");
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function is HAL API for SPI write.
*
* @param	Descriptor for the SPI driver.
* @param	Addr address to be written to.
* @param	Val value to write.
* @param	Len data length.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/

static int XRFClk_SPIWrData(XSpiPs *spi0, u8 *tx, u8 Len)
{

    int status;

    status= (int) XSpiPs_PolledTransfer(&Spi0, tx, NULL, Len);
    if (status == XST_FAILURE) {
		return XST_FAILURE;
	}

    return XST_SUCCESS;
}



/****************************************************************************/
/****************            A P I   section              *******************/
/****************************************************************************/

/****************************************************************************/
/**
*
* This function is used to write a register on one of LMX2594 or LMX04828.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d = {D2, D1, D0}
*	Where [D0,D1,D2] bits are:
*		LMK04282:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:21] - 2-bit multi-byte field (W1, W0)
*			bits [20:8] - 13-bit address field (A12 to A0)
*			bits [7-0]- 8-bit data field (D7 to D0).
*		LMX2594:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:16] - 7-bit address field (A6 to A0)
*			bits [15-0]- 16-bit data field (D15 to D0).
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_WriteReg(u32 ChipId, u32 d)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);

    u8 tx[3] = {(d >> 16) & 0xff, (d >> 8) & 0xff, d & 0xff};

    int Status = XSpiPs_SetSlaveSelect(&Spi0, ChipId);
    if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    return XRFClk_SPIWrData(&Spi0, tx, 3);

}


/****************************************************************************/
/**
*
* This function is used to read a register from one of LMX2594 or LMX04828.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d = {D2, D1, D0} input and output
*	Where [D0,D1,D2] bits are:
*		LMK04282:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:21] - 2-bit multi-byte field (W1, W0)
*			bits [20:8] - 13-bit address field (A12 to A0)
*			bits [7-0]- 8-bit data field (D7 to D0).
*		LMX2594:
*			bit [23] - 1-bit command field (R/W)
*			bits [22:16] - 7-bit address field (A6 to A0)
*			bits [15-0]- 16-bit data field (D15 to D0).
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_ReadReg(u32 ChipId, u32 *in, u32 *out)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);
	Xil_AssertNonvoid(in != NULL);

	u8 rx[3];
	u8 tx[3] = { 0xff & (*in >> 16), 0xff & (*in >> 8), 0xff & (*in) };

	/* Read register bit */
	tx[0] = tx[0] | RF_DATA_READ_BIT;

	int Status = XSpiPs_SetSlaveSelect(&Spi0, ChipId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (XST_SUCCESS != XRFClk_SPIRrData (&Spi0, tx, rx, 3)) {
		LOG("read reg");
		return XST_FAILURE;
	}

	*out = (rx[0] << 16) + (rx[1] << 8) + rx[2];


	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function is used to initialize RFCLK devices on i2c1-bus:
* i2c1 bus switch, i2c2spi bridge and MUX_SELx GPIOs.
*
* @param	none
*
* @return	GpioId gpio ID for Linux build, n/a for baremetal build.
*
* @note		None
*
****************************************************************************/

u32 XRFClk_Init(void)
{
	if (XST_FAILURE == XRFClk_InitGPIO()) {
		LOG("gpio init");
		return XST_FAILURE;
	}


	if (XST_FAILURE == XRFClk_InitSPI()) {
		LOG("spi init");
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to close RFCLK devices.
*
* @param	none
*
* @return	none
*
* @note		None
*
****************************************************************************/
void XRFClk_Close(void)
{

	XSpiPs_Disable(&Spi0);

}

/****************************************************************************/
/**
*
* This function is used to reset one of LMX2594 or LMK04828.
*
* @param	ChipId indicates the RF clock chip Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_ResetChip(u32 ChipId)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);

	u32 val = LMX_RESET_VAL;

	if (ChipId == RFCLK_LMK)
		val = LMK_RESET_VAL;

	if (XST_SUCCESS != XRFClk_WriteReg(ChipId, val)) {
		LOG("reset chip");
		return XST_FAILURE;
	}

	if (XST_SUCCESS != XRFClk_WriteReg(ChipId, 0)) {
		LOG("undo reset");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set config on LMK.
*
* @param	ConfigId indicates the config Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_SetConfigLMK(u32 ConfigId)
{
	for (int i = 0; i < LMK_COUNT; i++) {
		if (XST_SUCCESS !=
		    XRFClk_WriteReg(RFCLK_LMK, LMK_CKin[ConfigId][i])) {
			LOG("write reg in LMK");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set config on LMX.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	ConfigId indicates the config Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_SetConfigLMX(u32 ChipID, u32 ConfigId)
{

	for (int i = 0; i < LMX2594_COUNT; i++) {
		if (XST_SUCCESS !=
		    XRFClk_WriteReg(ChipID, LMX2594[ConfigId][i])) {
			LOG("write reg in LMX");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set a full configuration on one of LMX2594 or
* LMX04828 for the requested frequency.where the register settings is
* provided from the selected hard coded data.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	ConfigId indicates the RF clock chip configuration Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_SetConfigOnOneChipFromConfigId(u32 ChipId, u32 ConfigId)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);

	if (ChipId == RFCLK_LMK) {
		Xil_AssertNonvoid(ConfigId <
				  sizeof(LMK_CKin) / sizeof(LMK_CKin[0]));
		return XRFClk_SetConfigLMK(ConfigId);
	}
	Xil_AssertNonvoid(ConfigId < sizeof(LMX2594) / sizeof(LMX2594[0]));
	return XRFClk_SetConfigLMX(ChipId, ConfigId);
}

/****************************************************************************/
/**
*
* This function is used to set the full configuration data on one of
* LMX2594 or LMX04828. The all register values are passed as a pointer
* CfgData, Len defines a number of data ready for writing.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	CfgData indicates the configuration for all registers.
* @param	Len indicates a number of data.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_SetConfigOnOneChip(u32 ChipId, u32 *CfgData, u32 Len)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);
	Xil_AssertNonvoid(CfgData != NULL);
	Xil_AssertNonvoid(Len < 256);

	u32 *d = CfgData;
	for (u32 i = 0; i < Len; i++, d++) {
		if (XST_SUCCESS != XRFClk_WriteReg(ChipId, *d)) {
			LOG("write reg");
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to set a full configuration on all LMX2594
* and LMK04828 or LMK04208 for the requested frequency.
*
* @param	ConfigId_LMK indicates the LMK configuration Id.
* @param	ConfigId_RF1 indicates the LMX RF1 configuration Id.
* @param	ConfigId_RF2 indicates the LMX RF2 configuration Id.
* @param	ConfigId_RF3 indicates the LMX RF3 configuration Id.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_SetConfigOnAllChipsFromConfigId(u32 ConfigId_LMK, u32 ConfigId_1,
					   u32 ConfigId_2)
{
	Xil_AssertNonvoid(ConfigId_LMK < sizeof(LMK_CKin) / sizeof(LMK_CKin[0]));
	Xil_AssertNonvoid(ConfigId_1 < sizeof(LMX2594) / sizeof(LMX2594[0]));
	Xil_AssertNonvoid(ConfigId_2 < sizeof(LMX2594) / sizeof(LMX2594[0]));

	if (XST_SUCCESS !=
	    XRFClk_SetConfigOnOneChipFromConfigId(RFCLK_LMK, ConfigId_LMK)) {
		LOG("set config to LMK");
		return XST_FAILURE;
	}
	if (XST_SUCCESS != XRFClk_SetConfigOnOneChipFromConfigId(
				   RFCLK_LMX2594_1, ConfigId_1)) {
		LOG("set config to LMX1");
		return XST_FAILURE;
	}
	if (XST_SUCCESS != XRFClk_SetConfigOnOneChipFromConfigId(
				   RFCLK_LMX2594_2, ConfigId_2)) {
		LOG("set config to LMX2");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to get config from LMK
* It read all the LMK registers.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	in pointer to array of data read from the chip registers.
* @param	out pointer to array of data read from the chip registers.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_getConfig_fromLMK(u32 ChipId, u32 *in, u32 *out)
{

	int Status;

	// Ensure readback register is activated
	if (XST_SUCCESS != XRFClk_LMK_readback_activation(ChipId)) {
		LOG("reset chip");
		return XST_FAILURE;
	}


	for (int i = 0; i < LMK_COUNT; i++)
	{
		Status = XRFClk_ReadReg(ChipId, &in[i], &out[i]) ;
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to get config from LMK.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	d pointer to array of data read from the chip registers.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
static u32 XRFClk_getConfig_fromLMX(u32 ChipId, u32 *out)
{
	u32 in = 0;
	int Status;

	for (int i = 0; i < LMX2594_COUNT; i++) {

		in = (u32) i;

		Status = XRFClk_ReadReg(ChipId, &in, &out[i]) ;
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function is used to read the full configuration data from one of
* LMX2594 or LMX04828.
*
* @param	ChipId indicates the RF clock chip Id.
* @param	CfgData the array of the RF clock chip configuration data.
*
* @return
*	- XST_SUCCESS if successful.
*	- XST_FAILURE if failed.
*
* @note		None
*
****************************************************************************/
u32 XRFClk_GetConfigFromOneChip(u32 ChipId, u32 *CfgData)
{
	Xil_AssertNonvoid(ChipId < RFCLK_CHIP_NUM);
	Xil_AssertNonvoid(CfgData != NULL);

	u32 ret = XST_SUCCESS;
	u32 *d = CfgData;

	if (ChipId == RFCLK_LMK)
		ret = XRFClk_getConfig_fromLMK(ChipId, (u32 *const)LMK_CKin, d);
	else
		ret = XRFClk_getConfig_fromLMX(ChipId, d);

	return ret;
}


/** @} */
