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

void UI_ClearScreen(void);
void UI_DrawStatus(const char* apText);
void UI_DrawTitle(void);
void UI_DrawMenu(U8 aSelectedOption);
void UI_DrawGame(const char* apText);
void UI_DrawGameContent(const char* apText);
void UI_UpdateCursor(void);
void UI_ShowMessage(const char* apMessage);
void UI_ShowEndScreen(U8 aWin);

/* ################################################################################ */

#endif /* INCLUDED_UI_H */