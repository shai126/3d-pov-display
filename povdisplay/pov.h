/**
 * POV Display Library
 * Library for drawing to a cylindrical persistence of vision display
 * Call initializePov(), then override povDraw(uint8_t columnNo) to draw each column to the display
 * @author Shai Schechter @shaisc
 */

#pragma once

#include <stdint.h>
#include "Arduino.h"

/*****************************/
/*********LED DRAWING*********/
/*****************************/
#define NO_OF_RGB_LEDS  64

#ifndef NO_OF_COLUMNS
#define NO_OF_COLUMNS   80
#endif

#define NO_OF_BYTES_PER_COLOUR  (NO_OF_RGB_LEDS * 12 / 8)

/*****************************/
/*******PIN DEFINITIONS*******/
/*****************************/
// pin 2 - reserved for rotation-completed sensor

// pin 3 - should not be changed - timer pin!
#define GSCLK_DDR  DDRD
#define GSCLK_PORT PORTD
#define GSCLK_PIN  3

// pin 4
#define SELR_DDR   DDRD
#define SELR_PORT  PORTD
#define SELR_PIN   4

// pin 5
#define SELG_DDR   DDRD
#define SELG_PORT  PORTD
#define SELG_PIN   5

// pin 6
#define SELB_DDR   DDRD
#define SELB_PORT  PORTD
#define SELB_PIN   6

// pin 7
#define VPRG_DDR   DDRD
#define VPRG_PORT  PORTD
#define VPRG_PIN   7

// pin 8
#define DEBUG_DDR  DDRB
#define DEBUG_PORT PORTB
#define DEBUG_PIN  0

// pin 9
#define XLAT_DDR   DDRB
#define XLAT_PORT  PORTB
#define XLAT_PIN   1

// pin 10 - should not be changed - SPI pins!
#define BLANK_DDR  DDRB
#define BLANK_PORT PORTB
#define BLANK_PIN  2

// pin 11 - should not be changed - SPI pins!
#define SIN_DDR    DDRB
#define SIN_PORT   PORTB
#define SIN_PIN    3

// pin 13 - should not be changed - SPI pins!
#define SCLK_DDR   DDRB
#define SCLK_PORT  PORTB
#define SCLK_PIN   5



/*****************************/
/**********PIN MACROS*********/
/*****************************/
#define PIN_AS_OUTPUT(ddr, pin)   ddr  |=  _BV(pin)
#define PIN_AS_INPUT(ddr, pin)    ddr  &= ~_BV(pin)
#define PIN_HI(port, pin)         port |=  _BV(pin)
#define PIN_LO(port, pin)         port &= ~_BV(pin)
#define PIN_PULSE(port, pin)      do {                 \
                                    port |= _BV(pin);  \
                                    port &= ~_BV(pin); \
                                  } while(0)


/*****************************/
/***MISC DEFINITIONS/MACROS***/
/*****************************/
#define NOP   "NOP" "\n\t"
#define NOP2  NOP  NOP
#define NOP4  NOP2 NOP2
#define NOP8  NOP4 NOP4
#define NOP16 NOP8 NOP8

#define WAIT_FOR_SPI           while(!(SPSR & _BV(SPIF))) ;

#define IS_COMPL_ROTATION      (EIFR & _BV(INTF0))
#define ACK_COMPL_ROTATION     EIFR |= _BV(INTF0)


/*****************************/
/*****************************/

static void initializeTlcs()
{
  PIN_AS_OUTPUT(VPRG_DDR, VPRG_PIN);
  PIN_AS_OUTPUT(BLANK_DDR, BLANK_PIN);
  PIN_AS_OUTPUT(XLAT_DDR, XLAT_PIN);
  PIN_AS_OUTPUT(SELR_DDR, SELR_PIN);
  PIN_AS_OUTPUT(SELG_DDR, SELG_PIN);
  PIN_AS_OUTPUT(SELB_DDR, SELB_PIN);
  PIN_AS_OUTPUT(SIN_DDR, SIN_PIN);
  PIN_AS_OUTPUT(SCLK_DDR, SCLK_PIN);
  PIN_AS_OUTPUT(GSCLK_DDR, GSCLK_PIN);
  PIN_AS_OUTPUT(DEBUG_DDR, DEBUG_PIN);
  
  PIN_LO(SELR_PORT, SELR_PIN);
  PIN_LO(SELG_PORT, SELG_PIN);
  PIN_LO(SELB_PORT, SELB_PIN);
  
  PIN_LO(GSCLK_PORT, GSCLK_PIN);
  PIN_LO(SCLK_PORT, SCLK_PIN);
  PIN_LO(SIN_PORT, SIN_PIN);
  PIN_LO(XLAT_PORT, XLAT_PIN);
  PIN_HI(BLANK_PORT, BLANK_PIN);
  
  PIN_LO(VPRG_PORT, VPRG_PIN);
}

static void initializeRotationSensor()
{
  PIN_AS_INPUT(DDRD, 2); // pin 2 as input
  EICRA |= _BV(ISC01);   // pin 2 interrupt on falling edge (= magnet south pole -> north pole)
}

void clearAllLeds()
{
  uint16_t dataCounter = 0;

  PIN_HI(BLANK_PORT, BLANK_PIN);

  PIN_HI(SELR_PORT, SELR_PIN);
  PIN_HI(SELG_PORT, SELG_PIN);
  PIN_HI(SELB_PORT, SELB_PIN); 
  
  PIN_LO(SIN_PORT, SIN_PIN);
  for(dataCounter = 0; dataCounter < 64 * 12; dataCounter++)
      PIN_PULSE(SCLK_PORT, SCLK_PIN);
      
  PIN_LO(SELR_PORT, SELR_PIN);
  PIN_LO(SELG_PORT, SELG_PIN);
  PIN_LO(SELB_PORT, SELB_PIN);
}

static void enableSpi()
{
  SPCR = 0
       | _BV(SPE)
       | _BV(MSTR);
  SPSR = 0
       | _BV(SPI2X);
  
  SPDR = 0x00; // dummy SPI to set SPI transmission complete flag high
  WAIT_FOR_SPI
}

static void initializePwmClock()
{
  OCR2A  = 1;
  OCR2B  = 0;
  TCCR2A = 0
         | _BV(COM2B1)
         | _BV(WGM21)
         | _BV(WGM20);
  TCCR2B = 0
         | _BV(WGM22)
         | _BV(CS20);
}

static void initializeDataClock()
{
  TCCR0A = 0
         | _BV(WGM01);
  TCCR0B = 0
         | _BV(CS02)
         | _BV(CS00);
  OCR0A  = 7;
   
  TIMSK0 |= _BV(OCIE0A);
}

static void initializePov() {
  initializeTlcs();
  initializeRotationSensor();
  clearAllLeds();
  enableSpi();
  initializePwmClock();
  initializeDataClock();
}

// called every 4096 GSCLKs
// must complete in 4096 clock cycles or less
// and in this time must send the next column data down SIN/SCLK
ISR(TIMER0_COMPA_vect)
{ 
  static uint8_t columnNo = 0;
  
  if(IS_COMPL_ROTATION) {
    columnNo = 0;
    ACK_COMPL_ROTATION;
  }

  // just for debugging on scope
  //PIN_HI(DEBUG_PORT, DEBUG_PIN);

  static boolean isFirstCycle = true;
  static boolean xlatNeedsPulse = true;
  
  PIN_HI(BLANK_PORT, BLANK_PIN);
  
  if(xlatNeedsPulse)
  {
    PIN_PULSE(XLAT_PORT, XLAT_PIN);
    xlatNeedsPulse = false;
  }
  
  if(isFirstCycle)
  {
    PIN_PULSE(SCLK_PORT, SCLK_PIN);
    isFirstCycle = false;
  }
  
  PIN_LO(BLANK_PORT, BLANK_PIN);
  
  // OVERRIDE THIS METHOD in order to draw each column
  povDraw(columnNo);
  
  xlatNeedsPulse = true;
  
  // just for debugging on scope
  //PIN_LO(DEBUG_PORT, DEBUG_PIN);
  
  columnNo = (columnNo + 1) % NO_OF_COLUMNS;
}
