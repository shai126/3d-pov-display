/**
 * POV Display - Graphics Library
 * Graphics library to support drawing to a canvas for persistence of vision display
 * Currently supports pixel, line & circle drawing
 * @author Shai Schechter @shaisc
 */

#pragma once

#define POVG_DRAW_PIXEL(buffer, x, y)   buffer[(x)][(y) / 8] |= _BV((y) % 8)

static void povgDrawCircle(uint8_t (*bufferPtr)[8], uint8_t cx, uint8_t cy, uint8_t radius) {
  // Implementation of Bresenham's circle-drawing algorithm
  // Based loosely on lecture at http://www.cecs.csulb.edu/~pnguyen/cecs449/lectures/midpointcircle.pdf
  int8_t x = 0,
         y = radius,
         p = 5 - (4 * radius);
    
  while(x <= y) {
    POVG_DRAW_PIXEL(bufferPtr, cx + x, cy + y);
    POVG_DRAW_PIXEL(bufferPtr, cx - x, cy + y);
    POVG_DRAW_PIXEL(bufferPtr, cx + x, cy - y);
    POVG_DRAW_PIXEL(bufferPtr, cx - x, cy - y);
    POVG_DRAW_PIXEL(bufferPtr, cx + y, cy + x);
    POVG_DRAW_PIXEL(bufferPtr, cx - y, cy + x);
    POVG_DRAW_PIXEL(bufferPtr, cx + y, cy - x);
    POVG_DRAW_PIXEL(bufferPtr, cx - y, cy - x);
    
    x += 1;
    
    if(p < 0)
        p += (8 * x) + 4;
    else {
        y -= 1;
        p += (8 * (x - y)) + 4;
    }  
  }
}

static void povgDrawLine(uint8_t (*bufferPtr)[8], int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
  // Implementation of Bresenham's line-drawing algorithm
  // Based on algorithm at http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm, 21 Mar 2013
  // Licensed under GNU Free Documentation License 1.2
  int8_t dx  = abs(x1 - x0),
         sx  = (x0 < x1) ? 1 : -1;
  int8_t dy  = abs(y1 - y0),
         sy  = (y0 < y1) ? 1 : -1; 
  int8_t err = (dx > dy ? dx : -dy) / 2, errTemp;
 
  while(true) {
    POVG_DRAW_PIXEL(bufferPtr, x0, y0);
    
    if (x0 == x1 && y0 == y1)
      break;
    
    errTemp = err;
    if (errTemp > -dx) {
      err -= dy;
      x0  += sx;
    }
    if (errTemp < dy) {
      err += dx;
      y0  += sy;
    }
  }
}
