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

#include "engine.hh"
#include <MacMemory.h>
#include <Sound.h>
#include <Resources.h>
#include <QuickDraw.h>
#include <stdio.h>
#include <string.h>

Engine::Engine()
{
	dailyWords = GetResource('TEXT', 128);
	
	if (dailyWords == NULL)
	{
		SysBeep(1);
	}
	
	allWords_a = GetResource('TEXT', 129);
	
	if (allWords_a == NULL)
	{
		SysBeep(1);
	}
	
	allWords_b = GetResource('TEXT', 130);
	
	if (allWords_b == NULL)
	{
		SysBeep(1);
	}
	
	if (allWords_b != NULL)
	{
		memcpy(splitWord, *allWords_b, WORD_LENGTH);
		splitWord[WORD_LENGTH] = '\0';
	}
	
	newGame();
}

char* Engine::getSelectedWord()
{
	return selectedWord;
}

void Engine::newGame()
{
	*selectedWord = '\0';
	
	for (int i = 0; i < ALPHABET_LENGTH; i++)
	{
		alphabet[i] = Unknown;
	}
	
	numGuesses = 0;
	numCorrectLetters = 0;

	// Find the random number
	int randWordNum;
	randWordNum = Random() + -RAND_MIN;
	randWordNum = randWordNum % NUM_DAILY_WORDS;
	
	HLock(dailyWords);
	getWord(randWordNum, *dailyWords, selectedWord);
	HUnlock(dailyWords);
}

void Engine::getWord(int num, char* words, char* ret)
{
	for (int i = 0; i < WORD_LENGTH; i++)
	{
		ret[i] = words[(num * (WORD_LENGTH + 1)) + i];
	}

	ret[WORD_LENGTH] = '\0';
}

BOOL Engine::makeGuess(char* word)
{
	if(! checkWord(word))
	{
		return FALSE;
	}

	printf("Guess: %s, selected: %s\n", word, selectedWord);
	
	BOOL letterScore[WORD_LENGTH] = {FALSE};
	
	numCorrectLetters = 0;
	
	int i;

	// Default to NoMatch
	for (i = 0; i < WORD_LENGTH; i++)
	{
		scores[numGuesses][i] = NoMatch;
	}

	// Correct guesses
	for (i = 0; i < WORD_LENGTH; i++)
	{
		if (word[i] == selectedWord[i])
		{
			scores[numGuesses][i] = Correct;
			letterScore[i] = TRUE;
			numCorrectLetters++;
		}
	}

	// Wrong position but correct guess
	for (i = 0; i < WORD_LENGTH; i++)
	{
		char* foundLoc = strchr(selectedWord, word[i]);

		if (foundLoc && letterScore[foundLoc - selectedWord] == FALSE)
		{
			scores[numGuesses][i] = WrongPos;
			letterScore[foundLoc - selectedWord] = TRUE;
		}
	}
	
	for (i = 0; i < WORD_LENGTH; i++)
	{
		alphabetAdd(word[i], scores[numGuesses][i]);
		guesses[numGuesses][i] = word[i];
	}
	
	numGuesses++;

	return TRUE;
}

BOOL Engine::hasWon()
{
	return numCorrectLetters == WORD_LENGTH;
}

BOOL Engine::gameDone()
{
	return hasWon() || numGuesses == NUM_OF_GUESSES;
}

void Engine::alphabetAdd(char letter, letterScore score)
{
	printf("set letter %c to %d\n", letter, score);
	if (score > alphabet[letter - 'A'])
	{
		alphabet[letter - 'A'] = score;
	}
}

BOOL Engine::checkWord(char* word)
{
	BOOL retVal = FALSE;
	
	// First, always check the list of daily words
	
	HLock(dailyWords);
	retVal = binSearch(0, NUM_DAILY_WORDS, *dailyWords, word);
	HUnlock(dailyWords);
	
	if (retVal)
	{
		return retVal;
	}
	
	// Then, check the two auxilliary lists
	int cmpRes = strncmp(word, splitWord, WORD_LENGTH);
	printf("Comparing word %s to splitword %s results in: %d\n", word, splitWord, cmpRes);
	
	if (cmpRes < 0)
	{
		HLock(allWords_a);
		retVal = binSearch(0, NUM_WORDS_A, *allWords_a, word);
		HUnlock(allWords_a);
	}
	else if (cmpRes > 0)
	{
		HLock(allWords_b);
		retVal = binSearch(0, NUM_WORDS_B, *allWords_b, word);
		HUnlock(allWords_b);
	}
	else
	{
		retVal = TRUE;
	}
	
	return retVal;
}

BOOL Engine::binSearch(int start, int end, char* wordList, char* word)
{
	BOOL found = FALSE;
	// Base case, scan small lists
	if (end - start < 10)
	{
		for (int i = start; i < end; i++)
		{
			int res = memcmp(word, wordList + (i * (WORD_LENGTH + 1)), WORD_LENGTH);
			if(res == 0)
			{
				found = TRUE;
				break;
			}			
		}
		if (!found)
		{
			printf("Could not find word %s\n", word);
		}
	}
	else
	{
	
		// Recorsive case, choose a midpoint and test it
		int mid = ((end - start) / 2) + start;
	
		int cmpRes = memcmp(word, wordList + (mid * (WORD_LENGTH + 1)), WORD_LENGTH);
	
		if (cmpRes == 0)
		{
			found = TRUE;
		}
		else if (cmpRes < 0)
		{
			// Search the left side
			found = binSearch(start, mid, wordList, word);
		}
		else /* cmpRes > 0 */
		{
			// Search the right side
			found = binSearch(mid, end, wordList, word);
		}
	}
	
	return found;
}

void Engine::alphabetPrint()
{
	for (int i = 0; i < ALPHABET_LENGTH; i++)
	{
		printf("\tLetter %c: %d\n", 'A'+i, alphabet[i]);
	}
	return;
}
