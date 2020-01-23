/**
 * POV Display - Analog clock
 * Displays an analog clock on a cylindrical persistence of vision display
 * @author Shai Schechter @shaisc
 */
 

static void povDraw(uint8_t columnNo);
#include "/home/X09/schechax/povproject/pov.h"
#include "/home/X09/schechax/povproject/povGraphics.h"

uint8_t bufferOne[NO_OF_COLUMNS][NO_OF_RGB_LEDS / 8];
uint8_t bufferTwo[NO_OF_COLUMNS][NO_OF_RGB_LEDS / 8];
volatile boolean swapBufferReq = false, swapBufferAck = false;

#define HAND_SECOND  0
#define HAND_MINUTE  1
#define HAND_HOUR    2

#define START_HOUR   01
#define START_MINUTE 30
#define START_SECOND 00

const int8_t handEndPointOffsetMap[2][15][2] = {
  { // second hand, and divide by 2 for hour hand
    {  0, 22 }, {  2, 22 }, {  5, 22 }, {  7, 21 }, {  9, 20 },
    { 11, 19 }, { 13, 18 }, { 15, 16 }, { 16, 15 }, { 18, 13 },
    { 19, 11 }, { 20,  9 }, { 21,  7 }, { 22,  5 }, { 22,  2 }
  },
  { // minute hand
    {  0, 18 }, {  2, 18 }, {  4, 18 }, {  6, 17 }, {  7, 17 },
    {  9, 16 }, { 11, 15 }, { 12, 13 }, { 13, 12 }, { 15, 11 },
    { 16,  9 }, { 17,  7 }, { 17,  6 }, { 18,  4 }, { 18,  2 }
  }
};

static void povDraw(uint8_t columnNo)
{
  uint8_t i, j, temp;
  static uint8_t counter = 0;
  static uint8_t (*frontBufferPtr)[8] = bufferOne;
  
  if(columnNo == 79 && ++counter == 24) { // true every 1 second (ish)
    counter = 0;
    if(swapBufferReq) {
      frontBufferPtr = (frontBufferPtr == bufferOne) ? bufferTwo : bufferOne;
      swapBufferReq = false;
      swapBufferAck = true;
    }
  }
  
  // unpack the 64 x 1-bit pixels into
  // 64 x 12-bit words needed for the TLC drivers
  // but send it as 96 x 8-bit words for the SPI
  for(i = 0; i < NO_OF_RGB_LEDS / 8; i++) {
    for(j = 0; j < 8; j++) {
      temp = (frontBufferPtr[columnNo][i] & _BV(j)) ? 0xFF : 0x00;
      WAIT_FOR_SPI
      SPDR = temp;
      
      temp = (frontBufferPtr[columnNo][i] & _BV(j++)) ? 0xF0 : 0x00;
      temp |= (frontBufferPtr[columnNo][i] & _BV(j)) ? 0x0F : 0x00;
      WAIT_FOR_SPI
      SPDR = temp;
      
      temp = (frontBufferPtr[columnNo][i] & _BV(j)) ? 0xFF : 0x00;
      WAIT_FOR_SPI
      SPDR = temp;
    }
  }
}

void setup()
{
  initializePov();
  PIN_HI(SELR_PORT, SELR_PIN);
}

static void getHandEndPointOffset(uint8_t hand, uint8_t value, int8_t *returnX, int8_t *returnY) {
  
  const int8_t *a = handEndPointOffsetMap[hand][value % 15];
  
  // rotate the offset from the offset map into the correct quadrant
  switch(value / 15) {
    case 0:
      *returnX = -a[0];
      *returnY = -a[1];
      break;
    case 1:
      *returnX = -a[1];
      *returnY =  a[0];
      break;
    case 2:
      *returnX =  a[0];
      *returnY =  a[1];
      break;
    default:
      *returnX =  a[1];
      *returnY = -a[0];
      break;
  }
}

void loop()
{
  // code to draw next frame onto back buffer
  // and then wait for buffers to be swapped
  
  PIN_HI(DEBUG_PORT, DEBUG_PIN);
  
  static uint8_t (*backBufferPtr)[8] = bufferTwo;
  static uint8_t second = START_SECOND,
                 minute = START_MINUTE,
                 hour   = START_HOUR * 5; // time currently hard-coded. could come from e.g. DCF77 hardware
  uint8_t i = 0;
  int8_t x, y;
  
  // clear entire canvas
  memset(backBufferPtr, 0x00, sizeof(bufferOne));

  // draw clock face
  povgDrawCircle(backBufferPtr, 40, 32, 20);
  
  // (for now!) manually draw number 12 at top
  POVG_DRAW_PIXEL(backBufferPtr, 42, 1);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 0);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 1);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 2);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 3);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 4);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 5);
  POVG_DRAW_PIXEL(backBufferPtr, 41, 6);
  POVG_DRAW_PIXEL(backBufferPtr, 39, 1);
  POVG_DRAW_PIXEL(backBufferPtr, 38, 0);
  POVG_DRAW_PIXEL(backBufferPtr, 37, 0);
  POVG_DRAW_PIXEL(backBufferPtr, 36, 0);
  POVG_DRAW_PIXEL(backBufferPtr, 35, 1);
  POVG_DRAW_PIXEL(backBufferPtr, 35, 2);
  POVG_DRAW_PIXEL(backBufferPtr, 36, 3);
  POVG_DRAW_PIXEL(backBufferPtr, 37, 3);
  POVG_DRAW_PIXEL(backBufferPtr, 38, 3);
  POVG_DRAW_PIXEL(backBufferPtr, 39, 4);
  POVG_DRAW_PIXEL(backBufferPtr, 39, 5);
  POVG_DRAW_PIXEL(backBufferPtr, 38, 6);
  POVG_DRAW_PIXEL(backBufferPtr, 37, 6);
  POVG_DRAW_PIXEL(backBufferPtr, 36, 6);
  POVG_DRAW_PIXEL(backBufferPtr, 35, 6);

  getHandEndPointOffset(HAND_SECOND, second, &x, &y);
  POVG_DRAW_PIXEL(backBufferPtr, 40 + x, 32 + y);
  
  getHandEndPointOffset(HAND_MINUTE, minute, &x, &y);
  povgDrawLine(backBufferPtr, 40, 32, 40 + x, 32 + y);
  
  getHandEndPointOffset(HAND_SECOND, hour, &x, &y);
  povgDrawLine(backBufferPtr, 40, 32, 40 + x / 2, 32 + y / 2);
  
  if(++second == 60) {
    second = 0;
    if(++minute == 60) {
      minute = 0;
      hour += 5;
      if(hour == 60)
        hour = 0;
    }
  }
  
  PIN_LO(DEBUG_PORT, DEBUG_PIN);
  
  swapBufferReq = true;
  while(!swapBufferAck)
    ;
  swapBufferAck = false;
  backBufferPtr = (backBufferPtr == bufferOne) ? bufferTwo : bufferOne;
}

