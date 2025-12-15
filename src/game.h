/*************************************************************************************
 * Hangman ST - Game Logic Header
 *************************************************************************************
 */

#ifndef INCLUDED_GAME_H
#define INCLUDED_GAME_H

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include <godlib/base/base.h>

/* ###################################################################################
#  DEFINES
################################################################################### */

#define GAME_MAX_WORD_LEN    20
#define GAME_MAX_ALPHABET    64
#define GAME_ALPHABET_COLS   6
#define GAME_MAX_MISSES      12

/* ###################################################################################
#  ENUMS
################################################################################### */

enum {
    eGUESS_CORRECT = 0,
    eGUESS_WRONG,
    eGUESS_ALREADY_USED
};

enum {
    eDIR_UP = 0,
    eDIR_DOWN,
    eDIR_LEFT,
    eDIR_RIGHT
};

/* ###################################################################################
#  STRUCTS
################################################################################### */

typedef struct sGameState {
    char mSecret[GAME_MAX_WORD_LEN + 1];     /* Slowo do zgadniecia */
    char mAnswer[GAME_MAX_WORD_LEN + 1];     /* Aktualny stan (z '_') */
    char mAlphabet[GAME_MAX_ALPHABET + 1];   /* Dostepne litery */
    U8   mUsed[GAME_MAX_ALPHABET];           /* Ktore litery uzyte */
    U8   mMisses;                            /* Liczba bledow */
    U8   mGuesses;                           /* Liczba prob */
    U8   mSelectedLetter;                    /* Aktualnie wybrana litera */
    U8   mAlphabetLen;                       /* Dlugosc alfabetu */
    U8   mAlphabetCols;                      /* Kolumny w alfabecie */
    U8   mBoardSize;                         /* Rozmiar planszy (rows * cols) */
} sGameState;

/* ###################################################################################
#  GLOBALS
################################################################################### */

extern sGameState gGame;

/* ###################################################################################
#  PROTOTYPES
################################################################################### */

void Game_Init(void);
void InitRound(void);
U8   GuessLetter(void);
void MoveCursor(U8 aDirection);
U8   HasWon(void);
U8   HasLost(void);
void RevealAnswer(void);
S16   FindLetterIndex(char aChar);

/* ################################################################################ */

#endif /* INCLUDED_GAME_H */