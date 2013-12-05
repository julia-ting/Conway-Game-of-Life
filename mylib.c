#include "mylib.h"

unsigned short *videoBuffer = (unsigned short *)0x6000000;
unsigned short *drawBuffer = (unsigned short *)0x600A000;
unsigned short *displayBuffer = (unsigned short *)0x6000000;

CURSOR crs;

/*
***************
SET PIXEL!!
***************
*/
void setPixel4(int row, int col, volatile u8 index) {
  int pixel = OFFSET(row, col, 240);
  int shrt = pixel/2;

  if (col & 1) {
    //pixel is odd
    drawBuffer[shrt] = (drawBuffer[shrt] & 0x00FF) | (index<<8);
  } else {
    drawBuffer[shrt] = (drawBuffer[shrt] & 0xFF00) | (index);
  }
}


/*
*********
FLIP PAGE
*********
*/
void flipPage() {
  if (REG_DISPCTL & BUFFER1FLAG) {
    // We were displaying buffer 1, now we're not, so set to 0
    REG_DISPCTL = REG_DISPCTL & ~BUFFER1FLAG;
    drawBuffer = BUFFER1;
    displayBuffer = BUFFER0;
  } else {
    // We were displaying buffer 0, now we're not, set yes
    // to displaying buffer1 and buffer0 is drawing pad.
    REG_DISPCTL = REG_DISPCTL | BUFFER1FLAG;
    drawBuffer = BUFFER0;
    displayBuffer = BUFFER1;
  }
}

/*
***************
DMA FUNCTIONS
***************
*/
void drawRect4(int row, int col, int height, int width, unsigned char index) {
	int r;
	volatile unsigned short color = index | (index<<8);
	for (r=0; r<height; r++) {
		DMA[3].src = &color;
		DMA[3].dst = &drawBuffer[OFFSET(row+r, col, 240)/2];
		DMA[3].cnt = (width/2) | DMA_SOURCE_FIXED | DMA_ON;
	}
}

void waitForVBlank() {
  while (SCANLINECOUNTER > 160);
  while (SCANLINECOUNTER < 160);
}

void fillScreen4(unsigned char index) {
  volatile unsigned short color = index | (index<<8);
  DMA[3].src = &color;
  DMA[3].dst = drawBuffer;
  DMA[3].cnt = 19200 | DMA_SOURCE_FIXED | DMA_ON;
}

void drawImage4(int row, int col, int width, int height, const u16 *image) {
  for (int i =0; i < height; i++) {
    DMA[3].src = &image[width*i/2];
    DMA[3].dst = &drawBuffer[OFFSET(row+i, col, 240)/2];
    DMA[3].cnt = width/2 | DMA_SOURCE_INCREMENT | DMA_ON;
  }
}

/*
*****************
CURSOR FUNCTIONS
*****************
*/
// make a sim work
// THEN deal with edit

// Move cursor by x and y
void moveCursor(int x, int y) {
	eraseCursor();
	crs.oldRow = crs.row;
	crs.oldCol = crs.col;
	crs.row = crs.row + (x);
	crs.col = crs.col + (y);
	if (crs.row < 0) {
		crs.row = 0;
	} 
	if (crs.col < 0) {
		crs.col = 0;
	}
	if (crs.row >=HEIGHT) {
		crs.row = HEIGHT-1;
	}
	if (crs.col >= WIDTH) {
		crs.col = WIDTH-1;
	}
	crs.state = getPix(crs.row, crs.col);
}

// Hopefully erases the cursor lulzulzulzuzlululz
void eraseCursor() {
	if (isAlive(crs.oldRow, crs.oldCol)) {
		setPixel4(crs.oldRow, crs.oldCol, ALIVE);
	} else {
		setPixel4(crs.oldRow, crs.oldCol, DEAD);
	}
}

// Changes the state of the cursor
// Updates old values & erases old cursor
void toggleCursor() {
	eraseCursor();
	crs.oldRow = crs.row;
	crs.oldCol = crs.col;
	if (isAlive(crs.row, crs.col)) {
		crs.state = DEAD;
	} else {
		crs.state = ALIVE;
	}
}

// Draws the cursor according to the state
void drawCursor() {
	eraseCursor();
	if (crs.state) {
		setPixel4(crs.row, crs.col, CURSALIVE);
	} else {
		setPixel4(crs.row, crs.col, CURSDEAD);
	}
}


/*
*****************************
CONWAY GAME OF LIFE FUNCTIONS
*****************************
*/

// Returns the status (0, 1, 2, 3) of the pixel
// from the DISPLAY buffer
u8 getPix(int row, int col) {
  int pixel = OFFSET(row, col, 240);
  int shrt = pixel/2;
  int result;
  if (col & 1) {
    //pixel is odd
    result = displayBuffer[shrt];
    result = result>>8;
  } else {
    // gets the index
    result = displayBuffer[shrt];
  }
  return (u8)result;
}

int isAlive(int row, int col) {
	int state = getPix(row, col);
	return (state == ALIVE || state == CURSALIVE);
}
		

// Returns the state that it should be NEXT at this location
// 0 = dead
// 1 = alive
int checkNextPixelState(int row, int col) {
  int top = row-1;
  int liveCount = 0;
  int pixelState = isAlive(row, col);
  int currCol;
  int currPixel;

  if (top > 0 && top < 160) {
    for (int i =0; i < 3; i++) {
      currCol = col-1+i;
      if (currCol > 0 && currCol < 240) {
      // currPixel is the index color of the row & col
        currPixel = isAlive(top, currCol);
        // check what state currPixel is in alive/dead
        if (currPixel) {
        	liveCount++;
        }
      }
    }
  }
  int bot = row + 1;
  if (bot > 0 && bot < 160) {
    for (int i =0; i < 3; i++) {
      currCol = col-1+i;
      if (currCol > 0 && currCol < 240) {
      // currPixel is the index color of the row & col
        currPixel = isAlive(bot, currCol);
        // check what state currPixel is in alive/dead
        if (currPixel) {
        	liveCount++;
        }
      }
    }
  }
  if (row > 0 && row < 160) {
  	if (col-1 > 0 && col-1 < 240) {
  		currPixel = isAlive(row, col-1);
  		if (currPixel) {
  			liveCount++;
  		}
  	}
  	if (col+1 > 0 && col+1 < 240) {
  		currPixel = isAlive(row, col+1);
  		if (currPixel) {
  			liveCount++;
  		} 
  	}
  }
  
  // 1 is pixel is alive
  if (pixelState) {
    if (liveCount < 2 || liveCount > 3) {
      pixelState = 0;
    } else {
      pixelState = 1;
    }
  } else {
    if (liveCount == 3) {
      pixelState = 1;
    } else {
      pixelState = 0;
    }
  }
  return pixelState;
}

void drawScreen() {
	for (int r = 0; r < HEIGHT; r++) {
		for (int c = 0; c < WIDTH; c++) {
			setPixel4(r, c, checkNextPixelState(r,c));
		}
	}
}

void copy() {
	DMA[3].src = displayBuffer;
	DMA[3].dst = drawBuffer;
	DMA[3].cnt = 19200 | DMA_DESTINATION_INCREMENT | DMA_SOURCE_INCREMENT | DMA_ON;
}

// changes state of pixel depending on if  cursor is on it
void changePixel(int cursorRow, int cursorCol, int pixelRow, int pixelCol) {
	if (cursorRow == pixelRow && cursorCol == pixelCol) {
		// 1 is alive
		if (getPix(pixelRow, pixelCol) == 1) {
			setPixel4(cursorRow, cursorCol, REDIDX);
		} else {
			setPixel4(cursorRow, cursorCol, GREENIDX);
		}
	} else {
		if (getPix(pixelRow, pixelCol) == 1) {
			setPixel4(cursorRow, cursorCol, WHITEIDX);
		} else {
			setPixel4(cursorRow, cursorCol, BLACKIDX);
		}
	}
}

void setPalette(const u16 *pal) {
  DMA[3].src = pal;
  DMA[3].dst = PALETTE;
  DMA[3].cnt = 256 | DMA_ON;
}

