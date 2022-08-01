#ifndef ENGINE_HH
#define ENGINE_HH

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

#include <MacTypes.h>

#define WORD_LENGTH (5)
#define ALPHABET_LENGTH (26)
#define NUM_OF_GUESSES (6)
#define RAND_MIN (-32767)
#define RAND_MAX (32767)
#define FINAL_RAND_MAX (-RAND_MIN + RAND_MAX)
#define NUM_DAILY_WORDS (2309)
#define NUM_WORDS_A (5331)
#define NUM_WORDS_B (5332)

#define BOOL char
#define TRUE (1)
#define FALSE (0)

enum letterScore
{
	Unknown,
	NoMatch,
	WrongPos,
	Correct
};

class Engine
{
public:
	Engine();
	
	void alphabetPrint();
	
	BOOL makeGuess(char* word);
	
	BOOL hasWon();
	
	BOOL gameDone();
	
	char* getSelectedWord();
	
	void newGame();
	
	int numGuesses;
	char guesses[NUM_OF_GUESSES][WORD_LENGTH];
	letterScore scores[NUM_OF_GUESSES][WORD_LENGTH];
	letterScore alphabet[ALPHABET_LENGTH];
private:
	void alphabetAdd(char letter, letterScore score);
	void getWord(int num, char* words, char* ret);
	
	BOOL checkWord(char* word);
	
	BOOL binSearch(int start, int end, char* wordList, char* word);

	char selectedWord[WORD_LENGTH + 1];
	char splitWord[WORD_LENGTH + 1];
	
	int numCorrectLetters;
	
	Handle dailyWords;
	Handle allWords_a;
	Handle allWords_b;
};

#endif