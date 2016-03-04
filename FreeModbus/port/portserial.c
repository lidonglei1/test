/*
 * FreeModbus Libary: MSP430 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.3 2006/11/19 03:57:49 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define U0_CHAR                 ( 0x00 )        /* Data 0:8-bits / 1:7-bits */

#define DEBUG_PERFORMANCE       ( 1 )

#if DEBUG_PERFORMANCE == 1
#define DEBUG_PIN_RX            ( 0 )
#define DEBUG_PIN_TX            ( 6 )
#define DEBUG_PORT_DIR_RX       ( P1DIR )
#define DEBUG_PORT_DIR_TX       ( P4DIR )
#define DEBUG_PORT_OUT_RX       ( P1OUT )
#define DEBUG_PORT_OUT_TX       ( P4OUT )
#define DEBUG_INIT( )           \
  do \
  { \
    DEBUG_PORT_DIR_RX |= ( 1 << DEBUG_PIN_RX ) ; \
    DEBUG_PORT_OUT_RX &= ~( 1 << DEBUG_PIN_RX ) ; \
	DEBUG_PORT_DIR_TX |=  ( 1 << DEBUG_PIN_TX );\
	DEBUG_PORT_OUT_TX &= ~( 1 << DEBUG_PIN_TX ) ;\
 } while( 0 );
#define DEBUG_TOGGLE_RX( ) DEBUG_PORT_OUT_RX ^= ( 1 << DEBUG_PIN_RX )
#define DEBUG_TOGGLE_TX( ) DEBUG_PORT_OUT_RX ^= ( 1 << DEBUG_PIN_TX )

#else

#define DEBUG_INIT( )
#define DEBUG_TOGGLE_RX( )
#define DEBUG_TOGGLE_TX( )
#endif

/* ----------------------- Static variables ---------------------------------*/
UCHAR           ucGIEWasEnabled = FALSE;
UCHAR           ucCriticalNesting = 0x00;

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION(  );
    if( xRxEnable )
    {
        //IE1 |= URXIE0;
    	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
    }
    else
    {
        //IE1 &= ~URXIE0;
    	UCA0IE &= ~UCRXIE;                         // Enable USCI_A0 RX interrupt
    }
    if( xTxEnable )
    {
        //IE1 |= UTXIE0;
        //IFG1 |= UTXIFG0;

    	UCA0IE |= UCTXIE;                         // Enable USCI_A0 RX interrupt
    	UCA0IFG |= UCTXIFG;
    }
    else
    {
        //IE1 &= ~UTXIE0;
        UCA0IE &= ~UCTXIE;                         // Enable USCI_A0 RX interrupt
    }
    EXIT_CRITICAL_SECTION(  );
}

BOOL
xMBPortSerialInit( UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    BOOL            bInitialized = TRUE;
    USHORT          UCAxCTLW0 = 0;
//    USHORT          UCA0BR0 = ( USHORT ) ( SMCLK / ulBaudRate );
//    UCA0BRW = ( USHORT ) ( SMCLK / ulBaudRate );
      UCA0BR0 = 52;                             // 8000000/16/9600
      UCA0BR1 = 0x00;

    switch ( eParity )
    {
    case MB_PAR_NONE:
        break;
    case MB_PAR_ODD:
    	UCAxCTLW0 |= UCPEN;
        break;
    case MB_PAR_EVEN:
    	UCAxCTLW0 |= UCPEN | UCPAR;
        break;
    }
    switch ( ucDataBits )
    {
    case 8:
    	UCAxCTLW0 |= U0_CHAR;
        break;
    case 7:
        break;
    default:
        bInitialized = FALSE;
    }
    if( bInitialized )
    {
        ENTER_CRITICAL_SECTION(  );
        /* Reset USART */
//        U0CTL |= SWRST;


        /* Initialize all UART registers */
        //U0CTL = UxCTL | SWRST;
        UCA0CTLW0 = UCSWRST;

        /* SSELx = 11 = SMCLK. Use only if PLL is synchronized ! */
        //U0TCTL = SSEL1 | SSEL0;
        UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
        //U0RCTL = URXEIE;
        UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

        /* Configure USART0 Baudrate Registers. */
//        U0BR0 = ( UxBR & 0xFF );
//        U0BR1 = ( UxBR >> 8 );
//        U0MCTL = 0;
        UCA0BR0 = 52;                             // 8000000/16/9600
        UCA0BR1 = 0x00;
        UCA0MCTLW |= UCOS16 | UCBRF_1;

        /* Clear reset flag. */
        //U0CTL &= ~SWRST;
//        UCA0CTL0 &= ~UCSWRST;
        UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

        /* Enable UART */
//        UCA0IE |= UCTXIE | UCRXIE;
        UCA0IE |=  UCRXIE;


        /* USART0 TXD/RXD */
//        P3SEL |= 0x30;
//        P3DIR |= 0x10;
        P2SEL1 |= BIT0 | BIT1;                    // USCI_A0 UART operation
        P2SEL0 &= ~(BIT0 | BIT1);

        EXIT_CRITICAL_SECTION(  );

        DEBUG_INIT( );
    }
    return bInitialized;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
	UCA0TXBUF = ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = UCA0RXBUF;
    return TRUE;
}

//#pragma vector = USART0RX_VECTOR
//__interrupt
//void
// prvvMBSerialRXIRQHandler( void ) //__interrupt[USART0RX_VECTOR]
//{
//    DEBUG_TOGGLE_RX( );
//    pxMBFrameCBByteReceived(  );
//}
//#pragma vector = USART0TX_VECTOR
//__interrupt
//void
//prvvMBSerialTXIRQHandler( void )  // __interrupt[USART0TX_VECTOR]
//{
//    DEBUG_TOGGLE_TX( );
//    pxMBFrameCBTransmitterEmpty(  );
//}
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
//      while(!(UCA0IFG&UCTXIFG));
//      UCA0TXBUF = UCA0RXBUF;
//      __no_operation();

      DEBUG_TOGGLE_RX( );
      pxMBFrameCBByteReceived(  );
      break;
    case USCI_UART_UCTXIFG:

    	DEBUG_TOGGLE_TX( );
    	pxMBFrameCBTransmitterEmpty(  );

    	break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}

void
EnterCriticalSection( void )
{
    USHORT usOldSR;
    if( ucCriticalNesting == 0 )
    {
#if defined (__GNUC__)
        usOldSR = READ_SR;
        _DINT( );
#else
//        usOldSR = _DINT( );
#endif
//        ucGIEWasEnabled = usOldSR & GIE ? TRUE : FALSE;  
        _DINT( );  
        ucGIEWasEnabled = TRUE;
    }
    ucCriticalNesting++;
}

void
ExitCriticalSection( void )
{
    ucCriticalNesting--;
    if( ucCriticalNesting == 0 )
    {
        if( ucGIEWasEnabled )
        {
            ucGIEWasEnabled = FALSE;
            _EINT(  );
        }
    }
}
