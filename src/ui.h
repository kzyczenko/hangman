/*************************************************************************************
 * Hangman ST - UI Header
 *************************************************************************************
 */

#ifndef INCLUDED_UI_H
#define INCLUDED_UI_H

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include <godlib/base/base.h>

/* Stan aplikacji */
typedef struct sAppState {
    U8 mCurrentScreen;
    U8 mCurrentLevel;
    U8 mCurrentLang;
    U8 mQuit;
    U8 mExitApp;
} sAppState;

extern sAppState gApp;

/* ###################################################################################
#  PROTOTYPES
################################################################################### */

void ClearScreen(void);
void DrawStatus(const char* apText);
void DrawTitle(void);
void DrawMenu(U8 aSelectedOption);
void DrawGame(const char* apText);
void DrawGameContent(const char* apText);
void UpdateCursor(void);
void ShowEndScreen(U8 aWin);

/* ################################################################################ */

#endif /* INCLUDED_UI_H */