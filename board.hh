#ifndef BOARD_HH
#define BOARD_HH

/*
 * Copyright 2022 Trevor Gale
 *
 * This file is part of MacWords
 *
 * MacWords is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * MacWords is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with MacWords. If not,
 * see <https://www.gnu.org/licenses/>. 
 */

#include <MacWindows.h>
#include "engine.hh"
#include "windows.hh"

#define KEYBOARD_NUM_ROWS (3)

void c2pstrcpy_cust(Str255 dest, const char* src);

// Each board is pointed to by a window. Actions on performed on the board through the ui_main
class Board
{
	public:
		WindowType type;
		Board(WindowPtr w);
		void draw();
		void process_key(char key);
		void process_click(Point where);
		void newGame();
		void clear();
		void createScoreWindow(BOOL win);
		BOOL canCreateGWorld(short width, short numRows);
		void resized();
		~Board();
		
	private:
		void init();
		void cleanup();
		void draw_board();
		BOOL updateGWorld(const Rect* updateRect);
		Rect calculateVisibleRect(Rect r);
		
		void draw_letter(char letter, Rect r, short *fontSize, BOOL* updateFontSize);
		
		BOOL Board::equalPortRect(Rect cmp);
	
		Engine engine;
		WindowPtr window;
		
		PixPatHandle greyPixPat;
		PixPatHandle yellowPixPat;
		PixPatHandle greenPixPat;
		PixPatHandle lightGreyPixPat;
		
		Rect *key_rects[KEYBOARD_NUM_ROWS];
		int key_rect_len[KEYBOARD_NUM_ROWS];
		int max_key_rect_len;
		
		char curGuess[WORD_LENGTH + 1];
		unsigned char curGuessLen;
		Rect lastPortRect;

		short boardFontSize;
		short keyboardFontSize;
		BOOL boardSizeUpdated;
		BOOL keyboardSizeUpdated;
		
		BOOL redraw;
		
		GWorldPtr offscreenWorld;
};

#endif