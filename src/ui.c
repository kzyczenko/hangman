/*************************************************************************************
 * Hangman ST - UI (Text Mode) - FIXED DOUBLE BUFFERING
 *************************************************************************************
 */

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include "ui.h"
#include "game.h"
#include "dict.h"
#include <godlib/font8x8/font8x8.h>
#include <godlib/random/random.h>
#include <godlib/screen/screen.h>
#include <godlib/string/string.h>
#include <godlib/vbl/vbl.h>

/* ###################################################################################
#  DEFINES
################################################################################### */

#define CHAR_WIDTH   8
#define CHAR_HEIGHT  8

#define SCREEN_CHARS_X  40
#define SCREEN_CHARS_Y  25

#define ALPHABET_START_X  4
#define ALPHABET_START_Y  2
#define ALPHABET_SPACING  3

#define ANSWER_Y  16
#define HANGMAN_X  25
#define HANGMAN_Y  3

#define MESSAGE_Y  22
#define STATUS_Y   24

sAppState gApp;

/* ###################################################################################
#  PRIVATE FUNCTIONS
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION :  UI_PrintAt
 * ACTION   : Wypisz tekst na AKTUALNYM URZADZENIU LOGICZNYM
 *-----------------------------------------------------------------------------------*/

static void UI_PrintAt(const char* apText, U8 aX, U8 aY)
{
    Font8x8_Print(apText, Screen_GetpLogic(), aX * CHAR_WIDTH, aY * CHAR_HEIGHT);
}

/* ###################################################################################
#  PUBLIC FUNCTIONS
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_ClearScreen
 * ACTION   :  Wyczysc OBA ekrany
 *-----------------------------------------------------------------------------------*/

void UI_ClearScreen(void)
{
    Screen_Logic_ClearScreen();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawTitle
 * ACTION   : Rysuj tytul na obu buforach
 *-----------------------------------------------------------------------------------*/

void UI_DrawStatus(const char* apText)
{
    UI_PrintAt(apText, 0, STATUS_Y);
}

void UI_DrawTitle(void)
{
    UI_PrintAt("================================", 4, 2);
    UI_PrintAt("         H A N G M A N          ", 4, 4);
    UI_PrintAt("         ATARI ST PORT          ", 4, 6);
    UI_PrintAt("================================", 4, 8);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawMenu
 * ACTION   : Rysuj menu z zaznaczona opcja
 *-----------------------------------------------------------------------------------*/

void UI_DrawMenu(U8 aSelectedOption)
{
    U8 i, len;
    char tmpStr[50];
    extern U8 lSelectedLanguage;
    extern U8 lSelectedLevel;
    extern U8 lSelectedWordLen;

    for (i = 0; i < 5; i++) {
        U8 lY = 12 + (i * 2);

        switch(i) {
            case 0:
                String_StrCpy(tmpStr, gMessages[eMSG_COMMON].mStrings[0]); /* START GAME */
                break;
            case 1:
                if (gApp.mCurrentLevel & lvlTab[lSelectedLevel]) {
                    sprintf(tmpStr, "%s %d *", gMessages[eMSG_COMMON].mStrings[1], lSelectedLevel + 1); /* LEVEL */
                } else {
                    sprintf(tmpStr, "%s %d", gMessages[eMSG_COMMON].mStrings[1], lSelectedLevel + 1); /* LEVEL */
                }
                break;
            case 2:
            if (gApp.mCurrentLevel & (toTab[lSelectedWordLen] << 4)) {
                    sprintf(tmpStr, "%s %s *", gMessages[eMSG_COMMON].mStrings[2], lSelectedWordLen ? "13" : "7"); /* WORDS LENGTH */
                } else {
                    sprintf(tmpStr, "%s %s", gMessages[eMSG_COMMON].mStrings[2], lSelectedWordLen ? "13" : "7"); /* WORDS LENGTH */
                }
                break;
            case 3:
                sprintf(tmpStr, "%s (%s)", gMessages[eMSG_COMMON].mStrings[3], gDict.mLangs[lSelectedLanguage].mCode); /* LANGUAGE */
                break;
            case 4:
                String_StrCpy(tmpStr, gMessages[eMSG_COMMON].mStrings[4]); /* QUIT */
                break;
        }

        if (i == aSelectedOption) {
            UI_PrintAt(">>", 5, lY);
            UI_PrintAt(tmpStr, 8, lY);
            UI_PrintAt("<<", 30, lY);
        }
        else {
            UI_PrintAt("  ", 4, lY);
            UI_PrintAt(tmpStr, 8, lY);
            UI_PrintAt("  ", 30, lY);
        }
    }

    sprintf(tmpStr, "%s %d", gMessages[eMSG_COMMON].mStrings[5], Dict_GetWordCount());
    len = String_StrLen(tmpStr);
    UI_PrintAt(tmpStr, 20-(len/2), 24);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawAlphabet
 * ACTION   :  Rysuj alfabet z uzytymi literami
 *-----------------------------------------------------------------------------------*/

static void UI_DrawAlphabet(void)
{
    U8 i;
    U8 lX, lY;
    char lStr[2] = { 0, 0 };

    for (i = 0; i < gGame.mAlphabetLen; i++) {
        lX = ALPHABET_START_X + (i % gGame.mAlphabetCols) * ALPHABET_SPACING;
        lY = ALPHABET_START_Y + (i / gGame.mAlphabetCols) * 2;

        lStr[0] = gGame.mAlphabet[i];

        if (gGame.mUsed[i]) {
            lStr[0] = '.';
        }

        if (i == gGame.mSelectedLetter) {
            UI_PrintAt("[", lX - 1, lY);
            UI_PrintAt(lStr, lX, lY);
            UI_PrintAt("]", lX + 1, lY);
        }
        else {
            UI_PrintAt(" ", lX - 1, lY);
            UI_PrintAt(lStr, lX, lY);
            UI_PrintAt(" ", lX + 1, lY);
        }
    }
}
/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawAnswer
 * ACTION   : Rysuj haslo do odgadniecia
 *-----------------------------------------------------------------------------------*/

static void UI_DrawAnswer(void)
{
    U8 i;
    U8 lLen;
    U8 lStartX;
    char lStr[2] = { 0, 0 };

    UI_PrintAt("HAS£O:", 2, ANSWER_Y);

    lLen = 0;
    while (gGame.mAnswer[lLen] != '\0') lLen++;
    
    lStartX = (SCREEN_CHARS_X - (lLen * 2)) / 2;

    for (i = 0; i < lLen; i++) {
        lStr[0] = gGame.mAnswer[i];
        UI_PrintAt(lStr, lStartX + (i * 2), ANSWER_Y + 2);
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawHangman
 * ACTION   :  Rysuj wisielca (tekstowo)
 *-----------------------------------------------------------------------------------*/

static void UI_DrawHangman(void)
{
    U8 lMisses = gGame.mMisses;
    char lNum[4];

    UI_PrintAt("B£ÊDÓW:", HANGMAN_X, HANGMAN_Y);

    lNum[0] = '0' + (lMisses / 10);
    lNum[1] = '0' + (lMisses % 10);
    lNum[2] = '/';
    lNum[3] = '\0';
    UI_PrintAt(lNum, HANGMAN_X + 8, HANGMAN_Y);
    UI_PrintAt("12", HANGMAN_X + 11, HANGMAN_Y);

    UI_PrintAt(" +---+  ", HANGMAN_X, HANGMAN_Y + 2);
    UI_PrintAt(" |   |  ", HANGMAN_X, HANGMAN_Y + 3);

    if (lMisses >= 1) {
        UI_PrintAt(" |   O  ", HANGMAN_X, HANGMAN_Y + 4);
    } else {
        UI_PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 4);
    }

    if (lMisses >= 6) {
        UI_PrintAt(" |  /|\\ ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 5) {
        UI_PrintAt(" |  /|  ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 4) {
        UI_PrintAt(" |   |  ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 3) {
        UI_PrintAt(" |  \\|  ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 2) {
        UI_PrintAt(" |  \\   ", HANGMAN_X, HANGMAN_Y + 5);
    } else {
        UI_PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 5);
    }

    if (lMisses >= 7) {
        UI_PrintAt(" |   |  ", HANGMAN_X, HANGMAN_Y + 6);
    } else {
        UI_PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 6);
    }

    if (lMisses >= 12) {
        UI_PrintAt(" |  / \\ ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 11) {
        UI_PrintAt(" |  /   ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 10) {
        UI_PrintAt(" |    \\ ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 9) {
        UI_PrintAt(" |  |   ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 8) {
        UI_PrintAt(" |    | ", HANGMAN_X, HANGMAN_Y + 7);
    } else {
        UI_PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 7);
    }

    UI_PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 8);
    UI_PrintAt("===     ", HANGMAN_X, HANGMAN_Y + 9);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawGameContent
 * ACTION   : Rysuje zawarto¶æ ekranu gry (helper)
 *-----------------------------------------------------------------------------------*/

void UI_DrawGameContent(const char* apText)
{
    UI_ClearScreen();
        UI_PrintAt("=== HANGMAN ===", 12, 0);
        UI_DrawAlphabet();
        UI_DrawAnswer();
        UI_DrawHangman();
    UI_DrawStatus(apText);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_DrawGame
 * ACTION   : Rysuj caly ekran gry NA OBU BUFORACH
 *-----------------------------------------------------------------------------------*/

void UI_DrawGame(const char* apText)
{
    // U8 i;
    
    /* Rysuj na obu buforach */
    // for (i = 0; i < 2; i++) {
        UI_DrawGameContent(apText);
        Screen_Update();
        UI_DrawGameContent(apText);
        Screen_Update();
    // }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_UpdateCursor
 * ACTION   :  Aktualizuj tylko kursor NA OBU BUFORACH
 *-----------------------------------------------------------------------------------*/

void UI_UpdateCursor(void)
{
    UI_DrawAlphabet();
    Screen_Update();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_ShowMessage
 * ACTION   :  Pokaz wiadomosc NA OBU BUFORACH
 *-----------------------------------------------------------------------------------*/

void UI_ShowMessage(const char* apMessage)
{
    U8 i;
    U8 lLen;
    U8 lX;

    /* Oblicz d³ugo¶æ */
    lLen = 0;
    while (apMessage[lLen] != '\0') lLen++;
    lX = (SCREEN_CHARS_X - lLen) / 2;

    for (i = 0; i < 2; i++) {
        UI_PrintAt("                                        ", 0, MESSAGE_Y);
        UI_PrintAt(apMessage, lX, MESSAGE_Y);
        Screen_Update();
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UI_ShowEndScreen
 * ACTION   : Pokaz ekran konca gry
 *-----------------------------------------------------------------------------------*/

void UI_ShowEndScreen(U8 aWin)
{
    U8 lLen;
    U8 lX;
    U8 rnd;

    if (aWin) {
        // rnd = Random_GetClamped(gMessages[eMSG_WINS].mCount);
        rnd = Random_GetClamped(4);
        lLen = String_StrLen(gMessages[eMSG_WINS].mStrings[rnd]);
        lX = (40 - lLen) / 2;
        UI_PrintAt("================================", 4, 8);
        UI_PrintAt(gMessages[eMSG_WINS].mStrings[rnd], lX, 10);
        UI_PrintAt("================================", 4, 12);
    }
    else {
        // rnd = Random_GetClamped(gMessages[eMSG_LOOSES].mCount);
        rnd = Random_GetClamped(4);
        lLen = String_StrLen(gMessages[eMSG_LOOSES].mStrings[rnd]);
        lX = (40 - lLen) / 2;
        UI_PrintAt("================================", 4, 8);
        UI_PrintAt(gMessages[eMSG_LOOSES].mStrings[rnd], lX, 10);
        UI_PrintAt("================================", 4, 12);
    }
    /* Oblicz ¶rodek dla has³a */
    lLen = String_StrLen(gGame.mAnswer);
    lX = (40 - lLen) / 2;

    UI_PrintAt("HAS£O TO:", 15, 14);
    UI_PrintAt(gGame.mAnswer, lX, 16);

    // UI_DrawHangman();

    UI_PrintAt("NACI¦NIJ DOWOLNY KLAWISZ...", 6, 22);
}

/* ################################################################################ */