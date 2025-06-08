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
#include <Events.h>
#include <Dialogs.h>
#include <LowMem.h>
#include <ToolUtils.h>
#include <Sound.h>
#include <Devices.h>
#include <Menus.h>
#include <string.h>
#include <TextUtils.h>
#include <ctype.h>

#include <stdio.h>

#include "board.hh"
#include "windows.hh"

static const char programName[] = "MacWords";

static const short defaultMenubar = 128;
static const short appleMenu = 128;
static const short fileMenu = 129;
static const short editMenu = 130;
static const short gameMenu = 131;

static const short appleMenuAbout = 1;
static const short fileMenuQuit = 1;
static const short editMenuCopy = 1;
static const short editMenuClear = 2;
static const short gameMenuBegin = 1;
static const short gameMenuClose = 2;

#if __MWERKS__ == 0
QDGlobals qd;
#endif

short minHeight = 0;
short minWidth = 0;

void initialize();
void mainLoop();
void terminate();
void processMouseMenuEvent(long action);
void processKeyMenuEvent(char key);
void beginGame();
void newGame(WindowPtr window);
void clearEntry(WindowPtr window);
WindowPtr createAboutWindow();
void drawAboutWindow(WindowPtr window);
void closeAboutWinow(WindowPtr window);
void closeWindow(WindowPtr window);

static BOOL aboutWindowOpen;

void main()
{
	initialize();
	beginGame();
	mainLoop();
	terminate();
}

void initialize()
{
	MaxApplZone();

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	InitDialogs(nil);
	InitCursor();

	aboutWindowOpen = FALSE;

	FlushEvents(everyEvent, 0);
	qd.randSeed = TickCount();

	Handle menuBar = GetNewMBar(defaultMenubar);
	SetMenuBar(menuBar);
	AppendResMenu(GetMenuHandle(appleMenu), 'DRVR');
	DrawMenuBar();
}

void mainLoop()
{
	EventRecord event;
	while (true)
	{
		if (WaitNextEvent(everyEvent, &event, 10L, nil))
		{
			if (event.what == mouseDown)
			{
				WindowPtr clickedWindow;
				short clickedPort = FindWindow(event.where, &clickedWindow);
				if (clickedPort == inMenuBar)
				{
					processMouseMenuEvent(MenuSelect(event.where));
				}
				else if (clickedPort == inDrag)
				{
					SelectWindow(clickedWindow);
					DragWindow(clickedWindow, event.where, &qd.screenBits.bounds);					
				}
				else if (clickedPort == inGrow)
				{
					Rect r = {0};
					r.top = minHeight;
					r.bottom = 32767;
					r.left = minWidth;
					r.right = 32767;

					long newSize = GrowWindow(clickedWindow, event.where, &r);
					printf("inGrow. window %x becomes %lx\n", clickedWindow, newSize);

					Board* b = (Board*) GetWRefCon((WindowPtr) clickedWindow);

					BOOL canResize = b->canCreateGWorld(LoWord(newSize), HiWord(newSize));

					if (newSize && canResize)
					{
						SetPort(clickedWindow);

						// could try to resize here or calculate if memory is sufficient

						SizeWindow(clickedWindow, LoWord(newSize), HiWord(newSize), false);
						InvalRect(&clickedWindow->portRect);
						b->resized();
					}
					else
					{
						SysBeep(1);
					}
				}
				else if (clickedPort == inGoAway) {
					if (TrackGoAway(clickedWindow, event.where)) {
						if (clickedWindow)
						{
							closeWindow(clickedWindow);
						}
					}
				}
				else if (clickedPort == inContent)
				{
					if (clickedWindow != FrontWindow())
					{
						SelectWindow(clickedWindow);
					}
					else
					{
						if(clickedWindow)
						{
							Board* b = (Board*) GetWRefCon((WindowPtr) clickedWindow);

							if (b == 0)
							{
								// Close the About Window on click
								closeAboutWinow(clickedWindow);
							}

							// Process the click event
						}
					}
				}
			}
			else if (event.what == updateEvt)
			{
				printf("Doing an update event!!!!\n");
				BeginUpdate((WindowPtr) event.message);
				Board* b = (Board*) GetWRefCon((WindowPtr) event.message);
				if (b == 0)
				{
					drawAboutWindow((WindowPtr) event.message);
				}
				else if(b->type == BoardWindow)
				{
					RgnHandle rgn = NewRgn();
					//GetWindowRegion((WindowPtr) event.message, kWindowUpdateRgn, rgn);
					printf("The update region appears to be %x\n", rgn);
					b->draw();
					DisposeRgn(rgn);
				}
				else if(b->type == ScoreWindow)
				{
					Score* s = (Score*)b;
					s->draw();
				}

				EndUpdate((WindowPtr) event.message);
			}
			else if (event.what == keyDown)
			{
				if(((event.modifiers & cmdKey) != 0))
				{
					processKeyMenuEvent(tolower(LoWord(event.message)));
				}
				else
				{
					// Standard keyboard input
					Board* b = (Board*) GetWRefCon((WindowPtr) FrontWindow());

					if(b != NULL && b->type == BoardWindow)
					{
						char ch = LoWord(event.message);
						ch = toupper(ch);
						if (isalpha(ch) || ch == '\n' || ch == '\b')
						{
							b->process_key(ch);
							b->draw();
						}
					}
				}
			}
		}

	}
}

void terminate()
{
	ExitToShell();
}

void processMouseMenuEvent(long action)
{
	if (action <= 0)
	{
		return;
	}

	short menu = HiWord(action);
	short item = LoWord(action);

	if (menu == appleMenu)
	{
		if (item == appleMenuAbout)
		{
			SysBeep(1);
			WindowPtr aboutWindow = createAboutWindow();
			drawAboutWindow(aboutWindow);
		}
		else
		{
			Str255 name;
			GetMenuItemText(GetMenuHandle(menu), item, name);
			OpenDeskAcc(name);
		}
	}
	else if (menu == fileMenu)
	{
		HiliteMenu(fileMenu);
		if (item == fileMenuQuit)
		{
			terminate();
		}
	}
	else if (menu == editMenu)
	{
		HiliteMenu(editMenu);
		if (item == editMenuCopy)
		{
			// Copy
			SysBeep(1);
		}
		else if (item == editMenuClear)
		{
			// Clear
			clearEntry(FrontWindow());
		}
	}
	else if (menu == gameMenu)
	{
		HiliteMenu(gameMenu);
		if (item == gameMenuBegin)
		{
			newGame(FrontWindow());
		}
		else if (item == gameMenuClose)
		{
			closeWindow(FrontWindow());
		}
	}
	HiliteMenu(0);
}


void processKeyMenuEvent(char key)
{
	if (key == 'q')
	{
		HiliteMenu(fileMenu);
		terminate();
		HiliteMenu(0);
	}
	else if (key == 'c')
	{
		// Do copy
	}
	else if (key == 'b')
	{
		HiliteMenu(gameMenu);
		newGame(FrontWindow());
		HiliteMenu(0);
	}
	else if (key == 'w')
	{
		WindowPtr window = FrontWindow();
		HiliteMenu(gameMenu);
		closeWindow(window);
		HiliteMenu(0);
	}
	else if (key == 's') /* TODO: remove */
	{
		WindowPtr w = FrontWindow();
		Board* board = (Board*)GetWRefCon(w);
		if (board && board->type == BoardWindow)
		{
			board->createScoreWindow(TRUE);
		}
	}
}

void newGame(WindowPtr window)
{
	if(!window)
	{
		return;
	}

	Board* board = (Board*)GetWRefCon(window);

	if(board == 0)
	{
		return;
	}

	if(board->type != BoardWindow)
	{
		return;
	}

	board->newGame();
	board->draw();
}

void clearEntry(WindowPtr window)
{
	if(!window)
	{
		return;
	}

	Board* board = (Board*)GetWRefCon(window);

	if(board == 0)
	{
		return;
	}
	if(board->type != BoardWindow)
	{
		return;
	}

	board->clear();
	board->draw();
}

void beginGame()
{
	WindowPtr window = GetNewCWindow(mainWindow, nil, (WindowPtr) -1);
	DrawGrowIcon(window);
	Str255 title;
	c2pstrcpy_cust(title, programName);
	SetWTitle(window, title);

	Board* board = new Board(window);
	SetWRefCon(window, (long) board);
	SelectWindow(window);

	board->draw();
}

WindowPtr createAboutWindow()
{
	if (aboutWindowOpen)
	{
		return NULL;
	}

	WindowPtr window = GetNewCWindow(aboutWindow, nil, (WindowPtr) -1);
	SetWRefCon(window, (long) 0);

	aboutWindowOpen = TRUE;

	return window;
}

void drawAboutWindow(WindowPtr window)
{
	if (!window || !aboutWindowOpen)
	{
		return;
	}

	SetPort(window);

	RGBColor green;
	green.red = 93 << 8;
	green.green = 170 << 8;
	green.blue = 107 << 8;

	PixPatHandle greenPixPat = NewPixPat();
	MakeRGBPat(greenPixPat, &green);

	Rect r = window->portRect;

	FillCRect(&r, greenPixPat);

	RGBColor white;
	white.red = 255 << 8;
	white.green = 255 << 8;
	white.blue = 255 << 8;

	RGBForeColor(&white);

	short fontSize = 18;
	PenState oldState;
	GetPenState(&oldState);

	Style s = {0};
	Str255 fontName;

	short fontFamily = 0;
	c2pstrcpy_cust(fontName, "geneva");
	GetFNum(fontName, &fontFamily);

	TextFont(fontFamily);
	TextSize(fontSize);

	PenState ps = {0};
	ps.pnLoc.v = window->portRect.top + 20;
	ps.pnLoc.h = window->portRect.left + 5;
	ps.pnSize.v = 1;
	ps.pnSize.h = 1;

	SetPenState(&ps);

	Str255 writeString;
	c2pstrcpy_cust(writeString, programName);

	DrawString(writeString);

	c2pstrcpy_cust(writeString, " v1.2.0");

	DrawString(writeString);

	fontSize = 14;
	TextSize(fontSize);

	ps.pnLoc.v = ((window->portRect.bottom - window->portRect.top) / 2) + window->portRect.top;
	ps.pnLoc.h = window->portRect.left + 5;

	SetPenState(&ps);

	c2pstrcpy_cust(writeString, "Copyright 2024 Trevor Gale");

	DrawString(writeString);

	c2pstrcpy_cust(writeString, "under the GNU GPL");

	ps.pnLoc.v += 16;
	ps.pnLoc.h = window->portRect.left + 5;

	SetPenState(&ps);

	DrawString(writeString);

	DisposePixPat(greenPixPat);
}

void closeAboutWinow(WindowPtr window)
{
	if (!window || !aboutWindowOpen)
	{
		return;
	}

	DisposeWindow(window);
	aboutWindowOpen = FALSE;
}


void closeWindow(WindowPtr window)
{
	long board = 0;
	if (window)
	{
		board = GetWRefCon(window);
	}

	if (window && board && ((Board*)board)->type == BoardWindow)
	{
		terminate();
	}
	else if (window && board && ((Score*)board)->type == ScoreWindow)
	{
		Score* s = (Score*) board;
		delete s;
		DisposeWindow(window);
	}
	else if (window)
	{
		closeAboutWinow(window);
	}
}