/*****************************************************************************/
/**
*
* Author:      A. García-Requejo, A. Hernández
* Institution: University of Alcalá (UAH),
* 			   Electronics Department,
* 		       Geintra Research group
*
* Main example to control the LMK and LMX clks, configuring at xxxxx frequencies,
* for the RFSoC 4x2 board
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdarg.h>
#include "main.h"
#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xgpio.h"

#include "xrfdc.h"
#include "LMK_display.h"
#include "LMX_display.h"
#include "xrfclk.h"

#include <metal/log.h>
#include <metal/sys.h>

#include "platform.h"


/************************** Variable Definitions *****************************/

const char clkoutBrdNames[][18] = {
		"RFIN_RF1",   "RF1_ADC_SYNC",
		"NC",         "AMS_SYSREF",
		"RFIN_RF2",   "RF2_DAC_SYNC",
		"DAC_REFCLK", "DDR_PL_CAP_SYNC",
		"PL_CLK",     "PL_SYSREF",
		"NC",         "J10 SINGLE END",
		"ADC_REFCLK", "NC",
};

lmk_config_t lmkConfig;
lmx_config_t lmxConfig;

extern const u32 LMK_CKin[LMK_FREQ_NUM][LMK_COUNT];
extern const u32 LMX2594[LMX_ADC_NUM][LMX2594_COUNT];

/*
 * Device instance definitions
 */
XRFdc RFdcInst;      /* RFdc driver instance */

/*****************************************************************************/
/**
*
* Main function
*
* TBD
*
* @param	None
*
* @return
*		- XST_SUCCESS if tests pass
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	u32  Val;
	u32  Minor;
	u32  Major;
	int Status;
	XRFdc_Config *ConfigPtr;
    XGpio Gpio;

    init_platform();

    Status = init_GPIO_LedButtons (&Gpio);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

	// Initialize CLI commands structure
	xil_printf("\n\r###############################################\n\r");
	xil_printf("Hello RFSoC World!\n\r\n");

	// Display IP version
	Val = Xil_In32(XRFDC_BASE_ADDR + 0x00000);
	Major = (Val >> 24) & 0xFF;
	Minor = (Val >> 16) & 0xFF;

	xil_printf("RFDC IP Version: %d.%d\r\n",Major,Minor);

	Status = initAll_LMK_LMX();
	if (Status != XST_SUCCESS) {
	 	xil_printf("Failed to init LMK and LMXs\r\n");
	 	return XST_FAILURE;
	 	}

 	/* Initialize the RFdc driver. */
 	ConfigPtr = XRFdc_LookupConfig(RFDC_DEVICE_ID);
 	if (ConfigPtr == NULL) {
 		xil_printf("Failed to init RFdc driver\r\n");
 		return XST_FAILURE;
 	}else{
 		xil_printf("\n\rDeviceID: %d \r\nSilicon Revision: %d\r\n", ConfigPtr->DeviceId, ConfigPtr->SiRevision);

 	}

 	/* Initializes the controller */
 	Status = XRFdc_CfgInitialize(&RFdcInst, ConfigPtr);
 	if (Status != XST_SUCCESS) {
 		xil_printf("Failed to init RFdc controller\r\n");
 		return XST_FAILURE;
 	}
 	else {
 		xil_printf("The RFDC controller is initialized.\r\n");
 	}
 	// Display the Power-on Status
 	rfdcStartup();

    xil_printf("\n\r------------- Startup Complete ----------------\r\n\n");

    while (1)
    {
    	//on-off leads every 2 seconds
        XGpio_DiscreteWrite(&Gpio, 1, 0xFF);
        sleep(2);
        XGpio_DiscreteWrite(&Gpio, 1, 0x00);
        sleep(2);
    }

return 0;
}




/****************************************************************************/
/**
*
* This function .
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
int init_GPIO_LedButtons (XGpio *Gpio)
{
    // Initialize LED Device GPIO
    XGpio_Config *cfg_ptr;
    int Status;

    cfg_ptr = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
    if (cfg_ptr == NULL) {
        return XST_FAILURE;
    }
    Status = XGpio_CfgInitialize(Gpio, cfg_ptr, cfg_ptr->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    // Configurar como salida
    XGpio_SetDataDirection(Gpio, 1, 0x00);

    // On/off after 5 seconds
    XGpio_DiscreteWrite(Gpio, 1, 0xFF);
    sleep(5);
    XGpio_DiscreteWrite(Gpio, 1, 0x00);

    return EXIT_SUCCESS;

}


/****************************************************************************/
/**
*
* This function initialize the LMK and LMX clocks.
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
int initAll_LMK_LMX (void)
{

	// Configure RFSoC4x2 clks
	xil_printf("\nConfiguring the data converter clocks...\r\n");

	// initialize and reset _LMK_LMX devices via spi
	XRFClk_Init();

	// First of all is necessary to reset CLKs
	if (resetAll_LMK_LMX() == EXIT_FAILURE) {
		xil_printf("resetAll_LMK_LMX() failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Configuring _LMK_LMX LMK and LMX devices\r\n");

 	/* Set config on all chips */
 	// using below LMK and LMXs config indexes
 	if (XST_FAILURE == XRFClk_SetConfigOnAllChipsFromConfigId(LMK_CONFIG_INDEX, LMX_CONFIG_INDEX, LMX_CONFIG_INDEX)) {
 		xil_printf("Failure in XRFClk_SetConfigOnAllChipsFromConfigId()\n\r");
 		return XST_FAILURE;
 	}

 	// Print clock settings to the terminal
 	print_LMK_LMX_settings();

 	/* Close spi connections to _LMK_LMX */
 	XRFClk_Close();

 	return EXIT_SUCCESS;

}


/****************************************************************************/
/**
*
* This function resets all CLK_104 PLL I2C devices.
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
static int resetAll_LMK_LMX(void)
{
	int ret = EXIT_FAILURE;
//	printf("Reset LMK\n\r");
	if (XST_FAILURE == XRFClk_ResetChip(RFCLK_LMK)) {
		printf("Failure in XRFClk_ResetChip(RFCLK_LMK)\n\r");
		return ret;
	}

//	printf("Reset LMX2594_1\n\r");
	if (XST_FAILURE == XRFClk_ResetChip(RFCLK_LMX2594_1)) {
		printf("Failure in XRFClk_ResetChip(RFCLK_LMX2594_1)\n\r");
		return ret;
	}

//	printf("Reset LMX2594_2\n\r");
	if (XST_FAILURE == XRFClk_ResetChip(RFCLK_LMX2594_2)) {
		printf("Failure in XRFClk_ResetChip(RFCLK_LMX2594_2)\n\r");
		return ret;
	}


	return EXIT_SUCCESS;
}


/****************************************************************************/
/**
*
* Print LMK PLL device settings such as input and output clk frequencies.
* The instance structure is initialized by calling LMK_init()
*
* @param
*	- lmkInstPtr a pointer to the LMK instance structure
*
* @return
*	- void
*
* @note		None
*
****************************************************************************/
void printLMKsettings(lmk_config_t *lmkInstPtr)
{


    // Print LMK CLKin frequencies
    if(lmkInstPtr->clkin_sel_mode == LMK_CLKin_SEL_MODE_AUTO_MODE ) {
    	xil_printf("CLKin Auto Mode Enabled\n\r");
    }
    for(int i=0; i<3; i++) {
    	if(lmkInstPtr->clkin[i].freq != -1) {
    		xil_printf("CLKin%d_freq: %12ldKHz\n\r", i, lmkInstPtr->clkin[i].freq/1000);
    	}
    }

    // Print LMK CLKout frequencies
	for(int i=0; i<7; i++) {
		xil_printf("DCLKout%02d(%-10s):", i*2, clkoutBrdNames[i*2]);
		if(lmkInstPtr->clkout[i].dclk_freq == -1) {
			xil_printf("%12s", "-----");
		} else {
			xil_printf("%9ldKHz", lmkInstPtr->clkout[i].dclk_freq/1000);
		}

		xil_printf(" SDCLKout%02d(%-15s):", i*2 + 1, clkoutBrdNames[i*2 +1]);
		if(lmkInstPtr->clkout[i].sclk_freq == -1) {
			xil_printf("%12s\n\r", "-----");
		} else {
			xil_printf("%9ldKHz\n\r", lmkInstPtr->clkout[i].sclk_freq/1000);
		}
	}
}


/****************************************************************************/
/**
*
* Print LMX PLL device output clk frequencies.
* The instance structure is initialized by calling LMX_SettingsInit()
*
* @param
* 	- clkin is the clk freq fed into the LMX PLL. This value is used to
* 	  calculate and display the output frequencies
*	- lmxInstPtr a pointer to the LMX instance structure
*
* @return
*	- void
*
* @note		None
*
****************************************************************************/
void printLMXsettings(long int clkin, lmx_config_t *lmxInstPtr)
{


#ifdef LMX_DEBUG
    LMX_intermediateDump(lmxInstPtr);
#endif

    // Print LMX CLKin freq
    xil_printf("CLKin_freq: %10ldKHz\n\r", clkin/1000);


    // Print LMX CLKout frequencies
	xil_printf("RFoutA Freq:");
	if(lmxInstPtr->RFoutA_freq == -1) {
		xil_printf("%13s\n\r", "-----");
	} else {
		xil_printf("%10ldKHz\n\r", lmxInstPtr->RFoutA_freq/1000);
	}

	xil_printf("RFoutB Freq:");
	if(lmxInstPtr->RFoutB_freq == -1) {
		xil_printf("%13s\n\r", "-----");
	} else {
		xil_printf("%10ldKHz\n\r", lmxInstPtr->RFoutB_freq/1000);
	}

}


/****************************************************************************/
/**
*
* Reads the configuration of LMK and LMX PLL then calculates and displays
* the PLL frequencies and settings.
* The instance structures ar initialized by calling LMK_init() or
* LMX_SettingsInit()
*
* @param
* 	- nil
*
* @return
*	- void
*
* @note		None
*
****************************************************************************/
void print_LMK_LMX_settings(void)
{


#ifdef LMK_DEBUG
    LMK_intermediateDump(lmkInstPtr);
#endif

	// data buffer used for reading PLL registers
	u32 data[256];

	char pllNames[3][9] = {"LMK ----", "LMX_RF1", "LMX_RF2"};
	u32  chipIds[3] = {RFCLK_LMK, RFCLK_LMX2594_1, RFCLK_LMX2594_2};

	for(int i=0; i<3; i++) {
		if (XST_FAILURE == XRFClk_GetConfigFromOneChip(chipIds[i], data)) {
			printf("Failure in XRFClk_GetConfigFromOneChip()\n\r");
			return;
		}

		// For LMX, reverse readback data to match exported register sets and
		// order of LMX2594[][]
		if(chipIds[i] != RFCLK_LMK) {
			reverse32bArray(data, LMX2594_COUNT-3);
		}

#if 0
		// Dump raw data read from device
		printf("Config data is:\n\r");
		for (int j = 0; j < ((chipIds[i]==RFCLK_LMK) ? LMK_COUNT : LMX2594_COUNT-3); j++) {
			printf("%08X, ", data[j]);
			if( !(j % 6) ) printf("\n\r");
		}
		printf("\n\r");
#endif

		// Display clock values of device
		printf("Clk settings read from %s ---------------------\n\r", pllNames[i]);
		if(chipIds[i] == RFCLK_LMK) {
			LMK_init(data, &lmkConfig);
			printLMKsettings(&lmkConfig);
		} else {
			// clkout index is i=1 idx = 0, i=2 idx=2. i&2 meets this alg
			LMX_SettingsInit(lmkConfig.clkout[ (i & 2) ].dclk_freq, data, &lmxConfig);
			printLMXsettings(lmkConfig.clkout[ (i & 2) ].dclk_freq, &lmxConfig);		}
		xil_printf("\n\r");
	}
}



void reverse32bArray(u32 *src, int size) {
	u32 tmp[200];
	int i, j;

	//copy src into temp
	for(i = 0, j=size - 1; i < size; i++, j--) {
		tmp[i] = src[j];
	}

	//copy swapped array to original
	for(i=0; i< size; i++) {
		src[i] = tmp[i];
	}
	return;
}

/*****************************************************************************/
/**
*
* Startup DAC's and ADC's
*
* @param	None
*
* @return	None
*
* @note		TBD
*
******************************************************************************/
//void rfdcStartup (u32 *cmdVals) {
void rfdcStartup () {

	int Tile_Id;
	XRFdc_IPStatus ipStatus;
	XRFdc* RFdcInstPtr = &RFdcInst;
	u32 val;

	// Calling this function gets the status of the IP
	XRFdc_GetIPStatus(RFdcInstPtr, &ipStatus);

//	xil_printf("\r\n###############################################\r\n");
	xil_printf("Data Converter startup up is in progress...\n\r");

	// Master Reset
	Xil_Out32(XRFDC_BASE_ADDR + 0x0004, 1);

//	xil_printf("RF Data Converters Powered up.\r\n");
	sleep(1);

	// startup
	for ( Tile_Id=0; Tile_Id<=3; Tile_Id++) {
		if (ipStatus.DACTileStatus[Tile_Id].IsEnabled == 1) {
			val = XRFdc_ReadReg16(RFdcInstPtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id), XRFDC_ADC_DEBUG_RST_OFFSET);
			if(val & XRFDC_DBG_RST_CAL_MASK) {
				xil_printf("  Tile: %d NOT ready.\r\n", Tile_Id);
			} else {
				XRFdc_StartUp(RFdcInstPtr, 1, Tile_Id);
				usleep(200000);
			}
		}
	}

	for ( Tile_Id=0; Tile_Id<=3; Tile_Id++) {
		if (ipStatus.ADCTileStatus[Tile_Id].IsEnabled == 1) {
			val = XRFdc_ReadReg16(RFdcInstPtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id), XRFDC_ADC_DEBUG_RST_OFFSET);
			if(val & XRFDC_DBG_RST_CAL_MASK) {
				xil_printf("  ADC Tile%d NOT ready.\r\n", Tile_Id);
			} else {
				XRFdc_StartUp(RFdcInstPtr, 0, Tile_Id);
				usleep(200000);
			}
		}
	}

	xil_printf("\r\nThe Power-on sequence step. 0xF is complete.\r\n");


	for ( Tile_Id=0; Tile_Id<=3; Tile_Id++) {
		if (ipStatus.DACTileStatus[Tile_Id].IsEnabled == 1) {
			val = XRFdc_ReadReg16(RFdcInstPtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id), XRFDC_ADC_DEBUG_RST_OFFSET);
			if(val & XRFDC_DBG_RST_CAL_MASK) {
				xil_printf("  Tile: %d NOT ready.\r\n", Tile_Id);
			} else {
				xil_printf("   DAC Tile%d Power-on Sequence Step: 0x%08x\r\n",Tile_Id,
						Xil_In32(XRFDC_BASE_ADDR + 0x0000C + 0x04000 + Tile_Id * 0x4000));
			}
		}
	}

	for ( Tile_Id=0; Tile_Id<=3; Tile_Id++) {
		if (ipStatus.ADCTileStatus[Tile_Id].IsEnabled == 1) {
			val = XRFdc_ReadReg16(RFdcInstPtr, XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id), XRFDC_ADC_DEBUG_RST_OFFSET);
			if(val & XRFDC_DBG_RST_CAL_MASK) {
				xil_printf("  ADC Tile%d NOT ready.\r\n", Tile_Id);
			} else {
				xil_printf("   ADC Tile%d Power-on Sequence Step: 0x%08x\r\n",Tile_Id,
						Xil_In32(XRFDC_BASE_ADDR + 0x0000C + 0x14000 + Tile_Id * 0x4000));
			}
		}
	}


	xil_printf("\n\rData Converter start up is complete!");
	xil_printf("\r\n###############################################\r\n");

	return;
}


