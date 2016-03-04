/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430FR59xx Demo - eUSCI_A0 UART echo at 9600 baud using BRCLK = 8MHz
//
//  Description: This demo echoes back characters received via a PC serial port.
//  SMCLK/ DCO is used as a clock source and the device is put in LPM3
//  The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
//  when the UART is idle and turned on when a receive edge is detected.
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data.
//  To test code in LPM3, disconnect the debugger.
//
//  ACLK = VLO, MCLK =  DCO = SMCLK = 8MHz
//
//                MSP430FR5969
//             -----------------
//       RST -|     P2.0/UCA0TXD|----> PC (echo)
//            |                 |
//            |                 |
//            |     P2.1/UCA0RXD|<---- PC
//            |                 |
//
//   P. Thanigai
//   Texas Instruments Inc.
//   August 2012
//   Built with IAR Embedded Workbench V5.40 & Code Composer Studio V5.5
//******************************************************************************
#include "msp430.h"
#include "mb.h"
#include "mbport.h"
#include "mbutils.h"

typedef   signed short     int int16_t;
typedef unsigned short     int uint16_t;
typedef unsigned          char uint8_t;
//
#define REG_INPUT_START       0x0000
//
#define REG_INPUT_NREGS       8
//
#define REG_HOLDING_START     0x0000
//
#define REG_HOLDING_NREGS     8

//
#define REG_COILS_START       0x0000
//
#define REG_COILS_SIZE        16

//
#define REG_DISCRETE_START    0x0000
//
#define REG_DISCRETE_SIZE     16

/* Private variables ---------------------------------------------------------*/
//
uint16_t usRegInputBuf[REG_INPUT_NREGS] = {0x0ed6,0x1001,0x1002,0x1003,0x1004,0x1005,0x1006,0x1007};
//
uint16_t usRegInputStart = REG_INPUT_START;

//
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS] = {0x0ed6,0x3f8e,0x147b,0x400e,0x1eb8,0x4055,0x147b,0x408e};
//
uint16_t usRegHoldingStart = REG_HOLDING_START;

//
uint8_t ucRegCoilsBuf[REG_COILS_SIZE / 8] = {0x07,0x00};  //
//
uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8] = {0x01,0x02};

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog

  // Configure GPIO
  P2SEL1 |= BIT0 | BIT1;                    // USCI_A0 UART operation
  P2SEL0 &= ~(BIT0 | BIT1);

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // Startup clock system with max DCO setting ~8MHz
  CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers

//  // Configure USCI_A0 for UART mode
//  UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
//  UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
//  // Baud Rate calculation
//  // 8000000/(16*9600) = 52.083
//  // Fractional portion = 0.083
//  // User's Guide Table 21-4: UCBRSx = 0x04
//  // UCBRFx = int ( (52.083-52)*16) = 1
//  UCA0BR0 = 52;                             // 8000000/16/9600
//  UCA0BR1 = 0x00;
//  UCA0MCTLW |= UCOS16 | UCBRF_1;
//  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
//  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

  eMBInit(MB_RTU, 0x01, 0x01, 9600, MB_PAR_NONE);
  eMBEnable();
  _EINT();
  while(1)
	{
		eMBPoll();
	}
//  __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled
//  __no_operation();                         // For debugger
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
  eMBErrorCode    eStatus = MB_ENOERR;
  int16_t         iRegIndex;

  if( ( (int16_t)usAddress >= REG_INPUT_START ) \
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
  {

    iRegIndex = ( int16_t )( usAddress - usRegInputStart );//对应的地址

    while( usNRegs > 0 )
    {

      *pucRegBuffer++ = ( uint8_t )( usRegInputBuf[iRegIndex] >> 8 );

      *pucRegBuffer++ = ( uint8_t )( usRegInputBuf[iRegIndex] & 0xFF );

      iRegIndex++;

      usNRegs--;
    }
  }
  else
  {

    eStatus = MB_ENOREG;
  }

  return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{

  eMBErrorCode    eStatus = MB_ENOERR;

  int16_t         iRegIndex;


  if( ( (int16_t)usAddress >= REG_HOLDING_START ) \
     && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
  {

    iRegIndex = ( int16_t )( usAddress - usRegHoldingStart );

    switch ( eMode )
    {

      case MB_REG_READ:
        while( usNRegs > 0 )
        {
          *pucRegBuffer++ = ( uint8_t )( usRegHoldingBuf[iRegIndex] >> 8 );
          *pucRegBuffer++ = ( uint8_t )( usRegHoldingBuf[iRegIndex] & 0xFF );
          iRegIndex++;
          usNRegs--;
        }
        break;


      case MB_REG_WRITE:
        while( usNRegs > 0 )
        {
          usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
          usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
          iRegIndex++;
          usNRegs--;
        }
        break;
     }
  }
  else
  {

    eStatus = MB_ENOREG;
  }

  return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{

  eMBErrorCode    eStatus = MB_ENOERR;

  int16_t         iNCoils = ( int16_t )usNCoils;

  int16_t         usBitOffset;


  if( ( (int16_t)usAddress >= REG_COILS_START ) &&
        ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
  {

    usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
    switch ( eMode )
    {

      case MB_REG_READ:
        while( iNCoils > 0 )
        {
          *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
                                          ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
          iNCoils -= 8;
          usBitOffset += 8;
        }
        break;

      //
      case MB_REG_WRITE:
        while( iNCoils > 0 )
        {
//          xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
//                        ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),
//                        *pucRegBuffer++ );
        	ucRegCoilsBuf[usBitOffset] = *pucRegBuffer;

          iNCoils -= 8;
        }
        break;
    }

  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{

  eMBErrorCode    eStatus = MB_ENOERR;

  int16_t         iNDiscrete = ( int16_t )usNDiscrete;

  uint16_t        usBitOffset;


  if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&
        ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
  {

    usBitOffset = ( uint16_t )( usAddress - REG_DISCRETE_START );

    while( iNDiscrete > 0 )
    {
//      *pucRegBuffer++ = xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,
//                                      ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );

      *pucRegBuffer++ = ( uint8_t )( ucRegDiscreteBuf[usBitOffset] >> 8 );

      *pucRegBuffer++ = ( uint8_t )( ucRegDiscreteBuf[usBitOffset] & 0xFF );
      iNDiscrete -= 8;
      usBitOffset += 8;
    }

  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}

