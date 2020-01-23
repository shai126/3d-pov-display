/**
 * POV Display - multi-coloured text
 * Displays messages on a cylindrical persistence of vision display
 * Create custom fonts using my POV Converter app
 * @author Shai Schechter @shaisc
 */
 

#define NO_OF_COLUMNS   255
static void povDraw(uint8_t columnNo);
#include "/home/X09/schechax/povproject/pov.h"

///// text options ////
#define FONT_FILE        "/home/X09/schechax/povproject/povconverter/povfonts/basic.h"
#define MESSAGE_TEXT     "SPINNING"
///////////////////////

#define MESSAGE_LENGTH    sizeof(MESSAGE_TEXT) / sizeof(MESSAGE_TEXT[0]) - 1
uint8_t message[MESSAGE_LENGTH + 1] = MESSAGE_TEXT;

#define LETTER_TERM    0x0000, 0xFFFF
#include FONT_FILE




static void povDraw(uint8_t columnNo) {
  uint8_t i, temp;
  static uint8_t messageIndex, letterColumnIndex;
  
  // restart from beginning of message [actually end of message for English - spinning clockwise!]
  if(columnNo == 0) {
    messageIndex = MESSAGE_LENGTH - 1;
    letterColumnIndex = 0;
  }
  
  uint16_t letterColumnData = letterData[message[messageIndex]][letterColumnIndex];
  
  if(letterColumnData == 0xFFFF) { // letter terminator
    if(messageIndex == 0)
      // blank for remainder of cylinder (only show one instance of message)
      letterColumnData = 0x0000;
    else {
      // next letter
      messageIndex--;
      letterColumnIndex = 0;
      letterColumnData = letterData[message[messageIndex]][letterColumnIndex];
    }
  } else {
    // next column of current letter
    letterColumnIndex++; 
  }
  
  // pick a colour
  if(letterColumnIndex == 0) {
    PIN_LO(SELR_PORT, SELR_PIN);
    PIN_LO(SELG_PORT, SELG_PIN);
    PIN_LO(SELB_PORT, SELB_PIN);
    
    switch(messageIndex % 4) {
      case 0:
        PIN_HI(SELR_PORT, SELR_PIN);
      case 1:
        PIN_HI(SELG_PORT, SELG_PIN);
        break;
      case 2:
        PIN_HI(SELR_PORT, SELR_PIN);
      case 3:
        PIN_HI(SELB_PORT, SELB_PIN);
        break;
    }
  }
  
  // convert the 16 x 1-bit letter pixels
  // into 8-bit words for SPI
  i = 0;
  while(i < 16) {
    temp = (letterColumnData & _BV(i)) ? 0xFF : 0x00;
    WAIT_FOR_SPI
    SPDR = temp;
    
    temp = (letterColumnData & _BV(i++)) ? 0xF0 : 0x00;
    temp |= (letterColumnData & _BV(i)) ? 0x0F : 0x00;
    WAIT_FOR_SPI
    SPDR = temp;
    
    temp = (letterColumnData & _BV(i++)) ? 0xFF : 0x00;
    WAIT_FOR_SPI
    SPDR = temp;
  }
  i = 24;
  while(i < NO_OF_BYTES_PER_COLOUR) {
    WAIT_FOR_SPI
    SPDR = 0x00;
    i++;
  }
}




static void processMessage()
{
  // convert each character in message from ASCII to custom encoding
  // [0 = space, 1-26 = uppercase letters]
  uint8_t i = MESSAGE_LENGTH;
  while(i > 0) {
    i--;
    if(message[i] >= 0x41)
      message[i] -= 0x40;
    else
      message[i] = 0;
  }
}




void setup()
{
  processMessage();
  initializePov();
}

void loop()
{
  
}
