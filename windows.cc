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

#include <stdio.h>
#include <string.h>
#include <QuickDraw.h>

#include "board.hh"
#include "windows.hh"

Score::Score(WindowPtr w, BOOL win, char rounds, char* correctWord)
{
	type = ScoreWindow;
	window = w;
	didWin = win;
	numRounds = rounds;
	strcpy(word, correctWord);
	word[WORD_LENGTH] = '\0';
}

void Score::draw()
{
	SetPort(window);
	EraseRect(&window->portRect);
	
	// Scope for grey color
	{
		RGBColor macGrey;
		macGrey.red = 204 << 8;
		macGrey.green = 204 << 8;
		macGrey.blue = 204 << 8;
		
		PixPatHandle macGreyPixPat = NewPixPat();
		MakeRGBPat(macGreyPixPat, &macGrey);
		
		FillCRect(&window->portRect, macGreyPixPat);
		
		DisposePixPat(macGreyPixPat);
	}
	
	int squareSize = 32;
	int bufferSpace = 8;
	
	Rect topRightSquare;
	topRightSquare.right = window->portRect.right - bufferSpace;
	topRightSquare.left = topRightSquare.right - squareSize;
	topRightSquare.top = window->portRect.top + bufferSpace;
	topRightSquare.bottom = topRightSquare.top + squareSize;
	
	if (didWin)
	{
		RGBColor green;
		green.red = 93 << 8;
		green.green = 170 << 8;
		green.blue = 107 << 8;

		PixPatHandle greenPixPat = NewPixPat();
		MakeRGBPat(greenPixPat, &green);
		
		FillCRect(&topRightSquare, greenPixPat);
		
		DisposePixPat(greenPixPat);
	}
	else
	{
		RGBColor grey;
		grey.red = 119 << 8;
		grey.green = 124 << 8;
		grey.blue = 126 << 8;
		
		PixPatHandle greyPixPat = NewPixPat();
		MakeRGBPat(greyPixPat, &grey);
		
		FillCRect(&topRightSquare, greyPixPat);
		
		DisposePixPat(greyPixPat);
	}
	
	short winLoseFS = 24;
	PenState oldState;
	GetPenState(&oldState);
	
	Style s = {0};
	Str255 fontName;
	
	short fontFamily = 0;
	c2pstrcpy_cust(fontName, "geneva");
	GetFNum(fontName, &fontFamily);
	
	TextFont(fontFamily);
	TextSize(winLoseFS);
	
	PenState ps = {0};
	
	ps.pnLoc.v = window->portRect.top + 35;
	ps.pnLoc.h = window->portRect.left + 8;
	ps.pnSize.v = 1;
	ps.pnSize.h = 1;
	
	SetPenState(&ps);
	
	Str255 str;
	
	if (didWin)
	{
		c2pstrcpy_cust(str, "You Won!");
	}
	else
	{
		c2pstrcpy_cust(str, "You Lost :(");
	}
	
	DrawString(str);
	
	ps.pnLoc.v += 35;
	SetPenState(&ps);
	
	char recordString[255];
	sprintf(recordString, "Guesses: %d/%d", numRounds, NUM_OF_GUESSES);
	c2pstrcpy_cust(str, recordString);
	DrawString(str);
	
	ps.pnLoc.v += 35;
	SetPenState(&ps);
	
	TextSize(14);

	char correctWordString[255];
	sprintf(correctWordString, "The word was %s", word);
	c2pstrcpy_cust(str, correctWordString);
	DrawString(str);
	
	SetPenState(&oldState);
}