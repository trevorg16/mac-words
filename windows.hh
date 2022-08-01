#ifndef WINDOWS_HH
#define WINDOWS_HH

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
 
static const short mainWindow = 128;
static const short aboutWindow = 129;
static const short scoreWindow = 130;

enum WindowType
{
	BoardWindow,
	ScoreWindow
};

class Score
{
	public:
	WindowType type;
	
	Score(WindowPtr w, BOOL win, char rounds, char* correctWord);
	void draw();
	
	private:
	WindowPtr window;
	BOOL didWin;
	char numRounds;
	char word[WORD_LENGTH + 1];
};

#endif /* WINDOWS_HH */