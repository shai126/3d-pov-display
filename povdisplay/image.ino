/**
 * POV Display - Full colour images
 * Displays full colour images on a cylindrical persistence of vision display
 * Convert image to correct format using my POV Converter app
 * @author Shai Schechter @shaisc
 */
 
static void povDraw(uint8_t columnNo);
#include "/home/X09/schechax/povproject/pov.h"
#include <avr/pgmspace.h>

#include "/home/X09/schechax/povproject/povconverter/bartblue.h"

static void povDraw(uint8_t columnNo)
{ 
  // for each column, read the [already formatted to 8-bit] data from generated .h file above
  // and output it to R, G, B channels
  
  uint8_t i;
  const prog_uint8_t *dataPointer;

  // RED
  PIN_HI(SELR_PORT, SELR_PIN);
  asm volatile("ldi %0, %1\n\t" : "=r"(i) : "n"(NO_OF_BYTES_PER_COLOUR));
  dataPointer = dataRed[columnNo];
  do
  {
    SPDR = pgm_read_byte(dataPointer++);
    asm volatile(NOP8);
    --i;
  } 
  while(i > 0);
  PIN_LO(SELR_PORT, SELR_PIN);

  // GREEN
  PIN_HI(SELG_PORT, SELG_PIN);
  asm volatile("ldi %0, %1\n\t" : "=r"(i) : "n"(NO_OF_BYTES_PER_COLOUR));
  dataPointer = dataGreen[columnNo];
  do
  {
    SPDR = pgm_read_byte(dataPointer++);
    asm volatile(NOP8);
    --i;
  } 
  while(i > 0);
  PIN_LO(SELG_PORT, SELG_PIN);

  // BLUE
  PIN_HI(SELB_PORT, SELB_PIN);
  asm volatile("ldi %0, %1\n\t" : "=r"(i) : "n"(NO_OF_BYTES_PER_COLOUR));
  dataPointer = dataBlue[columnNo];
  do
  {
    SPDR = pgm_read_byte(dataPointer++);
    asm volatile(NOP8);
    --i;
  } 
  while(i > 0);
  PIN_LO(SELB_PORT, SELB_PIN);
}

void setup()
{
  initializePov();
}

void loop()
{
  
}

