// Julia Ting
// CS2110 HW11
// Conway's game of Life

#include "mylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "mainScreen.h"

extern const unsigned short mainScreen[19200];
extern const unsigned short mainScreen_palette[256];


int simTick();
int cursorMoves();
void delay(int n);

int cursorMoves() {
  int draw = 0;
  if (KEY_DOWN_NOW(BUTTON_UP)) {
	delay(1);
	moveCursor(-1, 0);
	draw = 1;
  } else if (KEY_DOWN_NOW(BUTTON_DOWN)) {
	delay(1);
	moveCursor(1, 0);
	draw = 1;
  } else if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
	delay(1);
  	moveCursor(0, 1);
	draw = 1;
  } else if (KEY_DOWN_NOW(BUTTON_LEFT)) {
	delay(1);
  	moveCursor(0,-1);
	draw = 1;
  } else if (KEY_DOWN_NOW(BUTTON_START)) {
	delay(1);
  	toggleCursor();
	draw = 1;
	while (KEY_DOWN_NOW(BUTTON_START)) {
	}
  }
  return draw;
}

int main() {

  REG_DISPCTL = MODE4 | BG2_ENABLE;
  
  // Splash screen here
  setPalette(mainScreen_palette);
  drawImage4(0, 0, 240, 160, mainScreen);
  waitForVBlank();
  flipPage();
  int screen = 1;
  
  int currentMode = EDITMODE;
  const u16 colors[] = {WHITE, BLACK, RED, GREEN};
  

  // alive = black = 1
  // dead = white = 0
  // alive + cursor = green = 3
  // dead + cursor = red = 2

  while (1) {
	  if (screen) {
	  	  setPalette(mainScreen_palette);
		  drawImage4(0, 0, 240, 160, mainScreen);
		  waitForVBlank();
		  flipPage();
	  	  if (KEY_DOWN_NOW(BUTTON_SELECT)) {
	  	  	screen = 0;
	  	    setPalette(colors);
  	  	    fillScreen4(DEAD);
  	  	    flipPage();
	  	    copy();
	  	  	while (KEY_DOWN_NOW(BUTTON_SELECT)) {
	  	  	}
	  	  }
	  
	  } else {
		  int draw = 0;
		  
		  // Select (backspace) = toggle between edit and simmode
	  	  if (KEY_DOWN_NOW(BUTTON_SELECT)) {
			if (currentMode == EDITMODE) {
			  currentMode = SIMMODE;
			  eraseCursor();
			} else {
			  currentMode = EDITMODE;
			}
			while (KEY_DOWN_NOW(BUTTON_SELECT)) {
			// Won't do anything while select button is down
			}
		  }
		  
		  // SIMMODE = 1
		  // EDITMODE = 0
		  
		  // ***************SIM MODE*******************
		  if (currentMode) {
		  	int paused = 0;
		  	int sim = 1;
		  	while (sim) {
		  		while (~paused & currentMode) {
			  		currentMode = simTick();
			  		if (KEY_DOWN_NOW(BUTTON_SELECT)) {
			  			currentMode = EDITMODE;
			  			sim = 0;
			  			while (KEY_DOWN_NOW(BUTTON_SELECT)) {
			  			}
			  		}
			  		if (KEY_DOWN_NOW(BUTTON_A)) {
			  			paused = 1;
			  			while (KEY_DOWN_NOW(BUTTON_A)) {
			  			}
			  		}
			  	}
		  		//******************PAUSED****************
			  	while (paused) {
		  	  		// Button B is X
	  				// This toggles next frame
			  		if (KEY_DOWN_NOW(BUTTON_B)) {
				  		simTick();
				  	}
					if (KEY_DOWN_NOW(BUTTON_SELECT)) {
			  			currentMode = EDITMODE;
			  			while (KEY_DOWN_NOW(BUTTON_SELECT)) {
			  			}
			  			sim = 0;
			  			paused = 0;
			  		}
			  		if (KEY_DOWN_NOW(BUTTON_A)) {
			  			paused = 0;
			  			while (KEY_DOWN_NOW(BUTTON_A)) {
			  			}
			  		}
			  	} // end of paused
		  	} // end of sim
		  } // end of currentMode if

		  // ************EDIT MODE**************
		  if (~currentMode) {
		  	copy();
		  	drawCursor();
		  	draw = cursorMoves();
		  	if (KEY_DOWN_NOW(BUTTON_L)) {
		  		fillScreen4(DEAD);
		  		flipPage();
		  		copy();
		  		while (KEY_DOWN_NOW(BUTTON_L)) {
		  		}
		  	}
		  }
		  
		  
		  drawCursor();
	  	  if (draw) {
			  waitForVBlank();
			  flipPage();
			  draw = 0;
		  }
	  } // end of else

  }

}

void delay(int n){
	int i;
	volatile int x=0;
	for(i = 0; i<n*10000; i++){
		x++;	
	}
}

int simTick() {
	if (KEY_DOWN_NOW(BUTTON_SELECT)) {
	  	while (KEY_DOWN_NOW(BUTTON_SELECT)) {
	  	}
	  	return 0;
	}
	drawScreen();
	waitForVBlank();
	flipPage();
	return 1;
}
