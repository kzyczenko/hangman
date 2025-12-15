/*************************************************************************************
 * Hangman ST - Game Logic (Updated to use Dictionary)
 *************************************************************************************
 */

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include "game.h"
#include "dict.h"
#include <godlib/string/string.h>

/* ###################################################################################
#  DATA
################################################################################### */

sGameState gGame;

/* ###################################################################################
#  CODE
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Game_Init
 * ACTION   : Jednorazowa inicjalizacja gry
 *-----------------------------------------------------------------------------------*/

void Game_Init(void)
{
    /* Pobierz alfabet ze słownika */
    const char* lpAlphabet = GetAlphabet();
    String_StrCpy(gGame.mAlphabet, lpAlphabet);
    gGame.mAlphabetLen = GetAlphabetLen();
    
    /* Ustaw liczbę kolumn w zależności od długości alfabetu */
    if (gGame.mAlphabetLen > 36) {
        gGame.mAlphabetCols = 7;
    } else {
        gGame.mAlphabetCols = 6;
    }
    
    /* Oblicz rozmiar planszy (zaokrąglony do pełnych wierszy) */
    U8 lRows = (gGame.mAlphabetLen + gGame.mAlphabetCols - 1) / gGame.mAlphabetCols;
    gGame.mBoardSize = lRows * gGame.mAlphabetCols;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : InitRound
 * ACTION   : Inicjalizacja nowej rundy
 *-----------------------------------------------------------------------------------*/

void InitRound(void)
{
    U8 i;
    U8 lLen;
    
    /* Wyzeruj stan */
    gGame.mMisses = 0;
    gGame.mGuesses = 0;
    gGame.mSelectedLetter = 0;
    
    /* Wyzeruj użyte litery */
    for (i = 0; i < GAME_MAX_ALPHABET; i++) {
        gGame.mUsed[i] = 0;
    }
    
    /* Załaduj nowy blok słów i pobierz losowe słowo */
    LoadRandomBlock();
    GetRandomWord(gGame.mSecret);
    
    /* Przygotuj odpowiedź (same podkreślniki) */
    lLen = String_StrLen(gGame.mSecret);
    for (i = 0; i < lLen; i++) {
        gGame.mAnswer[i] = '_';
    }
    gGame.mAnswer[lLen] = '\0';
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GuessLetter
 * ACTION   : Zgadnij aktualnie wybraną literę
 * RETURNS  : eGUESS_CORRECT, eGUESS_WRONG lub eGUESS_ALREADY_USED
 *-----------------------------------------------------------------------------------*/

U8 GuessLetter(void)
{
    char lChar;
    U8 i;
    U8 lLen;
    U8 lFound = 0;
    
    /* Sprawdź czy litera nie była już użyta */
    if (gGame.mUsed[gGame.mSelectedLetter]) {
        return eGUESS_ALREADY_USED;
    }
    
    /* Oznacz jako użytą */
    gGame.mUsed[gGame.mSelectedLetter] = 1;
    gGame.mGuesses++;
    
    /* Pobierz literę */
    lChar = gGame.mAlphabet[gGame.mSelectedLetter];
    
    /* Szukaj w słowie */
    lLen = String_StrLen(gGame.mSecret);
    for (i = 0; i < lLen; i++) {
        if (gGame.mSecret[i] == lChar) {
            gGame.mAnswer[i] = lChar;
            lFound = 1;
        }
    }
    
    if (lFound) {
        return eGUESS_CORRECT;
    }
    else {
        gGame.mMisses++;
        return eGUESS_WRONG;
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : MoveCursor
 * ACTION   :  Przesuń kursor w podanym kierunku
 *-----------------------------------------------------------------------------------*/

void MoveCursor(U8 aDirection)
{
    S8 lNewPos = gGame.mSelectedLetter;
    
    switch (aDirection) {
        case eDIR_UP:
            lNewPos -= gGame.mAlphabetCols;
            break;
        case eDIR_DOWN:
            lNewPos += gGame.mAlphabetCols;
            break;
        case eDIR_LEFT:
            lNewPos -= 1;
            break;
        case eDIR_RIGHT:
            lNewPos += 1;
            break;
    }
    
    /* Walidacja - zawiń jeśli poza zakresem */
    if (lNewPos < 0) {
        lNewPos += gGame.mBoardSize;
    }
    if (lNewPos >= gGame. mBoardSize) {
        lNewPos -= gGame.mBoardSize;
    }
    
    /* Upewnij się że nie wyszliśmy poza alfabet */
    if (lNewPos >= gGame.mAlphabetLen) {
        if (aDirection == eDIR_DOWN || aDirection == eDIR_RIGHT) {
            lNewPos = 0;
        }
        else {
            lNewPos = gGame.mAlphabetLen - 1;
        }
    }
    
    gGame.mSelectedLetter = lNewPos;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : HasWon
 * ACTION   :  Sprawdź czy gracz wygrał
 *-----------------------------------------------------------------------------------*/

U8 HasWon(void)
{
    U8 i;
    U8 lLen = String_StrLen(gGame.mAnswer);
    
    for (i = 0; i < lLen; i++) {
        if (gGame.mAnswer[i] == '_') {
            return 0;
        }
    }
    return 1;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : HasLost
 * ACTION   : Sprawdź czy gracz przegrał
 *-----------------------------------------------------------------------------------*/

U8 HasLost(void)
{
    return (gGame.mMisses >= GAME_MAX_MISSES);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : RevealAnswer
 * ACTION   : Odkryj odpowiedź (po przegranej)
 *-----------------------------------------------------------------------------------*/

void RevealAnswer(void)
{
    String_StrCpy(gGame.mAnswer, gGame.mSecret);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : FindLetterIndex
 * ACTION   : Znajdź indeks litery w alfabecie
 * RETURNS  : Indeks lub -1 jeśli nie znaleziono
 *-----------------------------------------------------------------------------------*/

S16 FindLetterIndex(char aChar)
{
    U8 i;
    
    /* Konwertuj małą literę na dużą */
    if (aChar >= 'a' && aChar <= 'z') {
        aChar = aChar - 'a' + 'A';
    }
    
    for (i = 0; i < gGame.mAlphabetLen; i++) {
        if (gGame.mAlphabet[i] == aChar) {
            return (S16)i;
        }
    }
    return -1;
}

/* ################################################################################ */