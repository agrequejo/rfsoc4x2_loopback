/*
 * main.h
 *
 *  Created on: Sep 17, 2017
 *      Author: 
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xrfdc.h"
#include "xgpio.h"


/******************** Constant Definitions **********************************/

// Necessary to use this define when using jtagterminal but not SDK jtaguart console
//#define STRIP_CHAR_CR

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
	#define RFDC_DEVICE_ID 	    XPAR_XRFDC_0_DEVICE_ID
	#define XRFDC_BASE_ADDR		XPAR_XRFDC_0_BASEADDR
	#define RFDC_DEV_NAME       XPAR_XRFDC_0_DEV_NAME
#else
	#define RFDC_DEVICE_ID      0
	#define XRFDC_BASE_ADDR		XPAR_XRFDC_0_BASEADDR
#endif

// Number of Tiles and Blocks in device
#define NUM_TILES 4
#define NUM_BLOCKS 4


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define LMK_CONFIG_INDEX	0
#define LMX_CONFIG_INDEX	0
#define LMX_CONFIG_INDEX	0

// PLL debug defines. Will print all calculated values
#undef LMK_DEBUG
#undef LMX_DEBUG

/************************** Function Prototypes *****************************/
static int resetAll_LMK_LMX(void);
static int initAll_LMK_LMX (void);
void reverse32bArray(u32 *src, int size);
void print_LMK_LMX_settings(void);
void rfdcStartup(void);

int init_GPIO_LedButtons (XGpio *Gpio);

/************************** Variable Definitions ****************************/

extern XRFdc RFdcInst;      /* RFdc driver instance */


#endif /* SRC_MAIN_H_ */
