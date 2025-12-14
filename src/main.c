/*************************************************************************************
 * Hangman ST - Port from Atari 8-bit
 * Original by Bocianu (MadPascal)
 * ST Port using godlib
 *
 * Skeleton version - text only, no sprites/graphics
 *************************************************************************************
 */

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include <godlib/gemdos/gemdos.h>
#include <godlib/ikbd/ikbd.h>
#include <godlib/platform/platform.h>
#include <godlib/screen/screen.h>
#include <godlib/font8x8/font8x8.h>
#include <godlib/vbl/vbl.h>
#include <godlib/video/video.h>
#include <godlib/memory/memory.h>
#include <godlib/string/string.h>
#include <godlib/file/file.h>
#include <godlib/random/random.h>

#include "game.h"
#include "input.h"
#include "ui.h"
#include "dict.h"

/* ###################################################################################
#  PROTOTYPES
################################################################################### */

void App_Init(void);
void App_DeInit(void);
void Game_Loop(void);
void TitleScreen_Run(void);
void GameScreen_Run(void);

/* ###################################################################################
#  DATA
################################################################################### */

/* Paleta kolorow - prosta dla trybu tekstowego */
U16 gPalette[16] = {
    0x000, 0xFFF, 0x0F0, 0xF00,  /* czarny, bialy, zielony, czerwony */
    0x00F, 0xFF0, 0x0FF, 0xF0F,  /* niebieski, zolty, cyan, magenta */
    0x888, 0xAAA, 0x080, 0x800,  /* szary, jasny szary, ciemny ziel, ciemny czerw */
    0x008, 0x880, 0x088, 0x808   /* ciemny nieb, oliwka, teal, purpura */
};

U8 lSelectedLanguage;
U8 lSelectedLevel;
U8 lSelectedWordLen;
U8 gFont8x8[(256-32)*8];

/* Ekrany */
enum {
    eSCREEN_TITLE = 0,
    eSCREEN_GAME
};

/* ###################################################################################
#  CODE
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GodLib_Game_Main
 * ACTION   : Entry point
 *-----------------------------------------------------------------------------------*/

S16 GodLib_Game_Main(S16 aArgCount, char* apArgs[])
{
    (void)aArgCount;
    (void)apArgs;

    GemDos_Super(0);
    Platform_Init();
    Random_Init();
    App_Init();

    Game_Loop();

    App_DeInit();
    Random_DeInit();
    Platform_DeInit();

    return 0;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION :  Load_Font8x8
 * ACTION   :  Inicjalizacja czcionki 8x8
 *-----------------------------------------------------------------------------------*/

void Load_Font8x8(void)
{
    /* Załaduj czcionkę 8x8 */
    U8 * font;
    font = File_Load("GFX\\FONT.FNT");
    Memory_Copy(2048-(32*8), font+(32*8), gFont8x8);
    File_UnLoad(font);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION :  App_Init
 * ACTION   :  Inicjalizacja aplikacji
 *-----------------------------------------------------------------------------------*/

void App_Init(void)
{
    Load_Font8x8();
    /* Inicjalizacja ekranu */
    Screen_Init(320, 200, eGRAPHIC_COLOURMODE_4PLANE, eSCREEN_SCROLL_NONE);
    Video_SetPalST(&gPalette[0]);

    /* Inicjalizacja podsystemow */
    Input_Init();
    Dict_Init();

    /* Stan poczatkowy */
    gApp.mCurrentScreen = eSCREEN_TITLE;
    gApp.mCurrentLevel = LVLALL; /* wszystkie poziomy */
    gApp.mCurrentLang = 0;
    gApp.mQuit = 0;
    gApp.mExitApp = 0;
    Dict_SetLang(gApp.mCurrentLang);

    /* Inicjalizacja gry */
    Game_Init();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : App_DeInit
 * ACTION   : Deinicjalizacja aplikacji
 *-----------------------------------------------------------------------------------*/

void App_DeInit(void)
{
    Dict_DeInit();
    Screen_DeInit();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Game_Loop
 * ACTION   :  Główna pętla aplikacji
 *-----------------------------------------------------------------------------------*/

void Game_Loop(void)
{
    /* Glowna petla aplikacji */
    while (! gApp.mExitApp) {
        TitleScreen_Run();

        if (! gApp.mExitApp) {
            gApp.mQuit = 0;

            /* Petla rozgrywek */
            while (!gApp.mQuit && !gApp.mExitApp) {
                GameScreen_Run();
            }
        }
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : TitleScreen_Run
 * ACTION   :  Ekran tytulowy
 *-----------------------------------------------------------------------------------*/

void TitleScreen_Run(void)
{
    U8 lSelectedOption = 0;
    U8 lCurrentLevel = gApp.mCurrentLevel;
    U8 lInput;
    U8 lRedraw = 1;

    gApp.mCurrentScreen = eSCREEN_TITLE;
    lSelectedLanguage = gApp.mCurrentLang;
    lSelectedLevel = 0;
    lSelectedWordLen = 0;

    while (1) {
        IKBD_Update();
        Input_Update();

        if (lRedraw) {
            UI_ClearScreen();
            UI_DrawTitle();
            UI_DrawMenu(lSelectedOption);
            lRedraw = 0;
            Screen_Update();
        }

        lInput = Input_GetMenu();

        switch (lInput)
        {
            case eINPUT_UP:
                if (lSelectedOption > 0) {
                    lSelectedOption--;
                    lRedraw = 1;
                }
                break;

            case eINPUT_DOWN:
                if (lSelectedOption < 4) {
                    lSelectedOption++;
                    lRedraw = 1;
                }
                break;

            case eINPUT_LEFT:
            case eINPUT_RIGHT:
                if (lSelectedOption == 1) { /* LEVEL */
                    lSelectedLevel =
                        (lSelectedLevel + 1) % 4;
                    lRedraw = 1;
                }
                else if (lSelectedOption == 2) { /* WORD LENGTH */
                    lSelectedWordLen =
                        (lSelectedWordLen + 1) % 2;
                    lRedraw = 1;
                }
                else if (lSelectedOption == 3) { /* LANGUAGE */
                    lSelectedLanguage =
                        (lSelectedLanguage + 1) & (DICT_MAX_LANGS - 1);
                    lRedraw = 1;
                }
                break;

            case eINPUT_FIRE:
                switch (lSelectedOption) {
                    case 0: /* START GAME */
                        return;

                    case 1: /* LEVEL */
                        lCurrentLevel ^= lvlTab[lSelectedLevel];
                        Dict_SetLevel(lCurrentLevel);
                        lRedraw = 1;
                        break;

                    case 2: /* WORD LENGTH */
                        lCurrentLevel ^= (U8)(toTab[lSelectedWordLen] << 4);
                        Dict_SetLevel(lCurrentLevel);
                        lRedraw = 1;
                        break;

                    case 3: /* LANGUAGE */
                        Dict_SetLang(lSelectedLanguage);
                        Game_Init();
                        lRedraw = 1;
                        break;

                    case 4: /* QUIT */
                        gApp.mExitApp = 1;
                        return;
                }
                break;

            case eINPUT_QUIT:
                gApp.mExitApp = 1;
                return;
        }
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GameScreen_Run
 * ACTION   : Glowna petla gry
 *-----------------------------------------------------------------------------------*/

void GameScreen_Run(void)
{
    U8 lInput;
    U8 lRedraw = 1;
    U8 lGameOver = 0;
    U8 lWin = 0;
    const char* message = gMessages[eMSG_PROMPT].mStrings[0];

    gApp.mCurrentScreen = eSCREEN_GAME;

    /* Nowa runda */
    Game_InitRound();
    UI_DrawGame(message);

    while (!lGameOver && !gApp.mQuit) {
        IKBD_Update();
        Input_Update();

        if (lRedraw) {
            UI_DrawGame(message);
            lRedraw = 0;
        }

        lInput = Input_GetGame();

        if (lInput == eINPUT_QUIT) {
            gApp.mQuit = 1;
        }
        else if (lInput == eINPUT_UP) {
            Game_MoveCursor(eDIR_UP);
            UI_UpdateCursor();
        }
        else if (lInput == eINPUT_DOWN) {
            Game_MoveCursor(eDIR_DOWN);
            UI_UpdateCursor();
        }
        else if (lInput == eINPUT_LEFT) {
            Game_MoveCursor(eDIR_LEFT);
            UI_UpdateCursor();
        }
        else if (lInput == eINPUT_RIGHT) {
            Game_MoveCursor(eDIR_RIGHT);
            UI_UpdateCursor();
        }
        else if (lInput == eINPUT_FIRE) {
            U8 lResult = Game_GuessLetter();
            if (lResult == eGUESS_CORRECT) {
                message = (gMessages[eMSG_CORRECT].mStrings[Random_GetClamped(gMessages[eMSG_CORRECT].mCount)]);
                /* TODO: dzwiek */
            }
            else if (lResult == eGUESS_WRONG) {
                message = (gMessages[eMSG_WRONG].mStrings[Random_GetClamped(gMessages[eMSG_WRONG].mCount)]);
                /* TODO: dzwiek */
            }
            /* lResult == eGUESS_ALREADY_USED - nic nie rob */
            lRedraw = 1;
        }
        else if (lInput >= 'A' && lInput <= 'Z') {
            /* Bezposrednie wcisniecie litery */
            S8 lLetterIndex = Game_FindLetterIndex((char)lInput);
            if (lLetterIndex >= 0) {
                gGame.mSelectedLetter = lLetterIndex;
                U8 lResult = Game_GuessLetter();
                if (lResult == eGUESS_CORRECT) {
                    message = (gMessages[eMSG_CORRECT].mStrings[Random_GetClamped(gMessages[eMSG_CORRECT].mCount)]);
                }
                else if (lResult == eGUESS_WRONG) {
                    message = (gMessages[eMSG_WRONG].mStrings[Random_GetClamped(gMessages[eMSG_WRONG].mCount)]);
                }
                lRedraw = 1;
            }
        }

        /* Sprawdz koniec gry */
        if (Game_HasWon()) {
            lGameOver = 1;
            lWin = 1;
        }
        else if (Game_HasLost()) {
            lGameOver = 1;
            lWin = 0;
        }
    }

    /* Koniec rundy */
    if (lGameOver) {
        UI_ClearScreen();
        if (lWin) {
            UI_ShowEndScreen(1);
        }
        else {
            Game_RevealAnswer();
            UI_ShowEndScreen(0);
        }
        Screen_Update();
        Input_WaitForAny();
    }
}

/* ################################################################################ */