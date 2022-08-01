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

#include "board.hh"
#include <Quickdraw.h>
#include <string.h>
#include <stdio.h>
#include <Sound.h>

#include "windows.hh"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define ROUND_RECT_SZ (5)

static char* keyboard[] = {"QWERTYUIOP", "ASDFGHJKL", "\nZXCVBNM\b", nil};

Board::Board(WindowPtr w)
{
	window = w;
	type = BoardWindow;

	lastPortRect.right = 0;
	lastPortRect.left = 0;
	lastPortRect.top = 0;
	lastPortRect.bottom = 0;

	boardFontSize = 0;
	keyboardFontSize = 0;

	init();
}

BOOL Board::equalPortRect(Rect cmp)
{
	return  cmp.right == lastPortRect.right &&
			cmp.left == lastPortRect.left &&
			cmp.top == lastPortRect.top &&
			cmp.bottom == lastPortRect.bottom;
}

Board::~Board()
{
	cleanup();
}

void Board::draw_board()
{
	BOOL isNewPortRect = !equalPortRect(window->portRect);
	boardSizeUpdated = isNewPortRect;
	keyboardSizeUpdated = isNewPortRect;
	short winWidth = window->portRect.right - window->portRect.left;
	
	float buttonWidth = ((float)winWidth) / (((float)max_key_rect_len) + 1);
	float spacerWidth = buttonWidth / (max_key_rect_len + 1);
	
	// Calculate space from the bottom
	
	short vertOffset = (window->portRect.bottom - window->portRect.top) - ((KEYBOARD_NUM_ROWS * (buttonWidth + spacerWidth) + spacerWidth));
	
	for (int row = 0; row < KEYBOARD_NUM_ROWS; row++)
	{
		for ( int col = 0; col < key_rect_len[row]; col++)
		{
			Rect r;
			r.top = vertOffset + (row * (buttonWidth + spacerWidth));
			r.bottom = r.top + buttonWidth;
			unsigned char numMissingKeys = max_key_rect_len - key_rect_len[row];
			r.left = spacerWidth + (spacerWidth + buttonWidth) * col + ((spacerWidth + buttonWidth) / 2) * numMissingKeys;
			r.right = r.left + buttonWidth;
			
			key_rects[row][col] = r;
			char l = keyboard[row][col];
			
			if ( l == '\n' )
			{
				FillCRoundRect(&r, ROUND_RECT_SZ, ROUND_RECT_SZ, lightGreyPixPat);
				l = 0xc8;
			}
			else if (l == '\b' )
			{
				FillCRoundRect(&r, ROUND_RECT_SZ, ROUND_RECT_SZ, lightGreyPixPat);
				l = 0xc7;
			}
			else
			{
				letterScore score = engine.alphabet[l - 'A'];
				if (score == Unknown)
				{
					FillCRoundRect(&r, ROUND_RECT_SZ, ROUND_RECT_SZ, lightGreyPixPat);
				}
				else if (score == NoMatch)
				{
					FillCRoundRect(&r, ROUND_RECT_SZ, ROUND_RECT_SZ, greyPixPat);
				}
				else if (score == WrongPos)
				{
					FillCRoundRect(&r, ROUND_RECT_SZ, ROUND_RECT_SZ, yellowPixPat);
				}
				else if (score == Correct)
				{
					FillCRoundRect(&r, ROUND_RECT_SZ, ROUND_RECT_SZ, greenPixPat);
				}
			}
			
			draw_letter(l, r, &keyboardFontSize, &keyboardSizeUpdated);
		}
	}
	
	float boardWidthCalc = (float)winWidth / ((float)WORD_LENGTH + 1.0);
	float boardHeightCalc = (float)vertOffset / ((float)NUM_OF_GUESSES + 1.0);
	float boardButtonWidth = MIN(boardWidthCalc, boardHeightCalc);
	float boardSpacerWidth = boardButtonWidth / (WORD_LENGTH + 1);
	
	float boardEmptySpace = winWidth - WORD_LENGTH * (boardButtonWidth + boardSpacerWidth);
	
	// Draw the game board
	for (int boardRow = 0; boardRow < NUM_OF_GUESSES; boardRow++)
	{
		for (int boardCol = 0; boardCol < WORD_LENGTH; boardCol++)
		{
			char letter = '\0';
			PixPatHandle color = nil;
			
			if (boardCol < curGuessLen && boardRow == engine.numGuesses)
			{
				letter = curGuess[boardCol];
			}
			else if (boardRow < engine.numGuesses )
			{
				letter = engine.guesses[boardRow][boardCol];
				
				letterScore score = engine.scores[boardRow][boardCol];
				
				if ( score == NoMatch)
				{
					color = greyPixPat;
				}
				else if (score == WrongPos)
				{
					color = yellowPixPat;
				}
				else if (score == Correct)
				{
					color = greenPixPat;
				}
			}
			
			Rect r;
			r.top = boardSpacerWidth + (boardButtonWidth + boardSpacerWidth) * boardRow;
			r.bottom = r.top + boardButtonWidth;
			r.left = boardSpacerWidth + (boardButtonWidth + boardSpacerWidth) * boardCol + (boardEmptySpace / 2.0);
			r.right = r.left + boardButtonWidth;
			
			if (color != nil)
			{
				FillCRect(&r, color);
			}
			else
			{
				FrameRect(&r);
			}


			if (letter != '\0')
			{
				draw_letter(letter, r, &boardFontSize, &boardSizeUpdated);
			}
		}
	}
}

void Board::draw_letter(char letter, Rect r, short* fontSize, BOOL* updateFontSize)
{
	PenState oldState;
	GetPenState(&oldState);

	if (*updateFontSize)
	{
		short fontSizes[] = { 72, 48, 36, 24, 18, 14, 12, 10, 9, 8, 7, 6, 5, 4};

		Style s = 0;
		Str255 fontName;
				
		// Use the courier monospace font
		short fontFamily = 0;
		c2pstrcpy_cust(fontName, "courier");
		GetFNum(fontName, &fontFamily);
		TextFont(fontFamily);
		for (int i = 0; i < (sizeof(fontSizes) / sizeof(fontSizes[0])); i++)
		{
			TextSize(fontSizes[i]);
			*fontSize = fontSizes[i];
		
			CharParameter testLetter = 'A';
			short charWidth = CharWidth(testLetter);
		
			FontInfo testInfo;
			GetFontInfo(&testInfo);
		
			if ( charWidth < (r.right - r.left) && ((testInfo.ascent + testInfo.descent) <= (r.bottom - r.top)))
			{
				break;
			}
		}
	
		*updateFontSize = FALSE;
	}
	else
	{
		Str255 fontName;
				
		// Use the courier monospace font
		short fontFamily = 0;
		c2pstrcpy_cust(fontName, "courier");
		GetFNum(fontName, &fontFamily);
		TextFont(fontFamily);

		TextSize(*fontSize);
	}
	
	CharParameter ch = letter;
	short space = ((r.right - r.left) - CharWidth(ch)) / 2;
	FontInfo info;
	GetFontInfo(&info);
	short vSpace = ((r.bottom - r.top) - (info.ascent + info.descent)) / 2;

	PenState ps = {0};
	ps.pnLoc.v = r.bottom - vSpace - info.descent;
	ps.pnLoc.h = r.left + space;
	ps.pnSize.v = 1;
	ps.pnSize.h = 1;

	SetPenState(&ps);
	DrawChar(ch);
	SetPenState(&oldState);
}

void Board::draw()
{
	SetPort(window);
	EraseRect(&window->portRect);
	draw_board();
}

void Board::process_key(char key)
{
	if (engine.gameDone())
	{
		// Already done, won't do anything
		SysBeep(1);
	}
	else if ( key == '\n')
	{
		// Enter case
		if ( curGuessLen == WORD_LENGTH )
		{
			// Always ensure the word is null terminated
			curGuess[WORD_LENGTH] = '\0';
			BOOL isValid = engine.makeGuess(curGuess);
			if (isValid)
			{
				curGuessLen = 0;
				
				// Check if the game is now won
				if (engine.hasWon())
				{
					createScoreWindow(TRUE);
				}
				else if (engine.gameDone())
				{
					createScoreWindow(FALSE);
				}
			}
			else
			{
				SysBeep(1);
			}
		}
		else
		{
			SysBeep(1);
		}
	}
	else if ( key == '\b')
	{
		// Backspace case
		if (curGuessLen > 0)
		{
			curGuessLen--;
		}
		else
		{
			SysBeep(1);
		}
	}
	else if (curGuessLen < WORD_LENGTH)
	{
		curGuess[curGuessLen] = key;
		curGuessLen++;
	}
	else
	{
		SysBeep(1);
	}
}

void Board::newGame()
{
	clear();
	engine.newGame();
}

void Board::clear()
{
	curGuessLen = 0;
}

void Board::init()
{
	
	curGuessLen = 0;

	RGBColor grey;
	grey.red = 119 << 8;
	grey.green = 124 << 8;
	grey.blue = 126 << 8;
	
	RGBColor yellow;
	yellow.red = 205 << 8;
	yellow.green = 178 << 8;
	yellow.blue = 100 << 8;
	
	RGBColor green;
	green.red = 93 << 8;
	green.green = 170 << 8;
	green.blue = 107 << 8;
	
	RGBColor lightGrey;
	lightGrey.red = 211 << 8;
	lightGrey.green = 214 << 8;
	lightGrey.blue = 218 << 8;
	
	greyPixPat = NewPixPat();
	yellowPixPat = NewPixPat();
	greenPixPat = NewPixPat();
	lightGreyPixPat = NewPixPat();
	
	MakeRGBPat(greyPixPat, &grey);
	MakeRGBPat(yellowPixPat, &yellow);
	MakeRGBPat(greenPixPat, &green);
	MakeRGBPat(lightGreyPixPat, &lightGrey);
	
	max_key_rect_len = 0;
	for (int i = 0; i < KEYBOARD_NUM_ROWS; i++)
	{
		int len = strlen(keyboard[i]);
		key_rect_len[i] = len;
		key_rects[i] = new Rect[len];
		
		if ( len > max_key_rect_len)
		{
			max_key_rect_len = len;
		}
	}
}

void Board::cleanup()
{
	DisposePixPat(greyPixPat);
	DisposePixPat(yellowPixPat);
	DisposePixPat(greenPixPat);
	DisposePixPat(lightGreyPixPat);
}

void c2pstrcpy_cust(Str255 dest, const char* src)
{
    unsigned char len = strlen(src);
	
	if(len >= 255)
	{
		len = 254;
	}

    dest[0] = len;
    memcpy(dest + 1, src, len);
}

void Board::createScoreWindow(BOOL win)
{
	
	WindowPtr window = GetNewCWindow(scoreWindow, nil, (WindowPtr) -1);
	Str255 title;
	c2pstrcpy_cust(title, "MacWords Score");
	SetWTitle(window, title);

	Score* score = new Score(window, win, (char)engine.numGuesses, engine.getSelectedWord());
	SetWRefCon(window, (long)score);

	score->draw();
}