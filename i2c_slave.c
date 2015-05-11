/*******************************************************************************
  I2C funtions
  
  Company:
    Microchip Technology Inc.

  File Name:
  i2c_slavedrv.c

  Summary:
    This file is used to configure I2C.

  Description:
    This code example shows how to use I2C module in slave mode.
 The master I2C device uses the slave I2C device as RAM.
 Thus master I2C device can read/write particular RAM area of I2C slave device.
*******************************************************************************/
/*******************************************************************************
Copyright (c) 2012 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <xc.h>
#include <stdint.h>
#include "i2c_slave.h"
#include "adc.h"
#include "control.h"


/*****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************/
//uint8_t         ramBuffer[256]; //RAM area which will work as EEPROM for Master I2C device
//uint8_t         *ramPtr;        //Pointer to RAM memory locations


#define MAX_LENGTH 16


 char txbuffer[MAX_LENGTH];
 uint8_t txtail=0;
uint8_t txhead=0;
 uint8_t txstate=0;


//struct FlagType flag;
uint8_t I2C_state=0;
uint8_t I2C_command;
uint8_t I2C_high,I2C_low;

// *****************************************************************************
// Section: Function definition
// *****************************************************************************
// *****************************************************************************

/******************************************************************************
 * Function:       void I2C1_Init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes I2C1 peripheral.
 *****************************************************************************/
void I2C1_Init( void )
{
    #if !defined( USE_I2C_Clock_Stretch )
    I2C1CON = 0x8000;       //Enable I2C1 module
    #else
    I2C1CON = 0x9040;       //Enable I2C1 module, enable clock stretching
    #endif
    I2C1ADD = 0x50;         // 7-bit I2C slave address must be initialised here.
    IFS1 = 0;
    //ramPtr = &ramBuffer[0]; //set the RAM pointer and points to beginning of ramBuffer
    //flag.AddrFlag = 0;      //Initlize Addflag
    // flag.DataFlag = 0;      //Initlize Dataflag
    uint8_t I2C_state=0;
    _SI2C1IE = 1;

}

/******************************************************************************
 * Function:   void __attribute__((interrupt,no_auto_psv)) _SI2C1Interrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This is the ISR for I2C1 Slave interrupt.
 *****************************************************************************/

// state machine 0 nothing
 // 1 address received
// 2 command received
// 3 first  byte received
// 4->0 second byte received
// 11 read address received
// 20 transmit first byte
// 21 transmit second byte
// 27           seventh byte


void __attribute__ ( (interrupt, no_auto_psv) ) _SI2C1Interrupt( void )
{
    unsigned char   temp;   //used for dummy read
    uint8_t i;
    uint16_t value;

    if( (I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0) )    //Address matched
    {
        temp = I2C1RCV;     //dummy read
        //flag.AddrFlag = 1;  //next byte will be address
        I2C_state =1;
    }
    else if( (I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1) )   //check for data
    {
        
            #if defined( USE_I2C_Clock_Stretch )
            I2C1CONbits.SCLREL = 1;                 //Release SCL1 line
            #endif


            temp = I2C1RCV;
            switch (I2C_state)
            {
                   case 1:
            // here comes the command;
                I2C_command =temp;
                if(I2C_command >=97 ) // a..z set command
                   I2C_state=2;
                else
                    I2C_state=11;
            break;
            case 2:
                I2C_high =temp; // ascii to int  conversion need to be done
                I2C_state=3;
            break;
            case 3:
                I2C_low =temp; // ascii to int  conversion need to be done
                I2C_state=0;
                value =I2C_low + 256 * I2C_high;

                SPIC.target=value;

            break;
            }
#ifdef NIX

            *ramPtr = ( unsigned char ) I2C1RCV;    // store data into RAM
            flag.AddrFlag = 0;                      //end of tx
            flag.DataFlag = 0;
            ramPtr = &ramBuffer[0];
#endif            //reset the RAM pointer
            #if defined( USE_I2C_Clock_Stretch )
            I2C1CONbits.SCLREL = 1;                 //Release SCL1 line
            #endif
        
    }
    else if( (I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0) )
    {
        temp = I2C1RCV;

        if(( I2C_command <97 )  && (I2C_state == 11 ) )
        {
            switch( I2C_command)
            {

                case 0x47: // debug

                for( i=0; i<8; i++ )
                {
                    if (txhead != txtail)
                    {
                        I2C1TRN = txbuffer[txhead];      //Read data from RAM & send data to I2C master device
                        I2C1CONbits.SCLREL = 1; //Release SCL1 line
                        while( I2C1STATbits.TBF );
                        txhead++;
                        if( txhead == MAX_LENGTH)
                            txhead = 0;
                    }
                    else{
                        if (txstate == 0)
                        {
                        I2C1TRN = txbuffer[txhead];      //Read data from RAM & send data to I2C master device
                        I2C1CONbits.SCLREL = 1; //Release SCL1 line
                        while( I2C1STATbits.TBF );
                    //U1STAbits.UTXEN = 0
                    // last char out
                        txstate = 1;
                        }
                        else {
                            I2C1TRN = 0xff;      //Read data from RAM & send data to I2C master device
                            I2C1CONbits.SCLREL = 1; //Release SCL1 line
                            while( I2C1STATbits.TBF );
                            // shut up sending
                            txstate = 0;
                        }
                    }


                }
                break;

                    default:
                        I2C1TRN = I2C_high;      //Read data from RAM & send data to I2C master device
                        I2C1CONbits.SCLREL = 1; //Release SCL1 line
                        while( I2C1STATbits.TBF );

                        I2C1TRN = I2C_low;      //Read data from RAM & send data to I2C master device
                        I2C1CONbits.SCLREL = 1; //Release SCL1 line
                        while( I2C1STATbits.TBF );
                     break;
             }

        }
        I2C_state=0;
        //Wait till all
        //ramPtr = &ramBuffer[0]; //reset the RAM pointer
    }

    _SI2C1IF = 0;               //clear I2C1 Slave interrupt flag
}


void I2Cprint(char *source, int len) {
    int i;


    for (i = 0; i < len; i++) {
        txbuffer[txtail++] = source[i];
        if (txtail == MAX_LENGTH)
            txtail = 0;
    }

}




/*******************************************************************************
 End of File
*/




