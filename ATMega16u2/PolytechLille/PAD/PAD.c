/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the RelayBoard program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "PAD.h"


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
    {
        USB_USBTask();
        
        /* Select the Keyboard Report Endpoint 
        Endpoint_SelectEndpoint(PAD_IN_EPADDR_JOYSTICK);

        /* Check if Keyboard Endpoint Ready for Read/Write and if we should send a new report 
        if (Endpoint_IsReadWriteAllowed() && Endpoint_IsINReady())
        {
            /* Save the current report data for later comparison to check for changes 
            //PrevKeyboardReportData = KeyboardReportData;

            /* Write Keyboard Report Data 
            Endpoint_Write_8(0xAA);

            /* Finalize the stream transfer to send the last packet 
            Endpoint_ClearIN();
        }*/
    }
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif

	/* Hardware Initialization */
	USB_Init();
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the keyboard device endpoints.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	/* Setup PAD Report Endpoints */
	Endpoint_ConfigureEndpoint(PAD_IN_EPADDR_JOYSTICK, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
    Endpoint_ConfigureEndpoint(PAD_IN_EPADDR_BUTTON, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(PAD_OUT_EPADDR, EP_TYPE_INTERRUPT, PAD_EPSIZE, 1);
}

