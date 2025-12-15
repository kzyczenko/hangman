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
#include "lang.h"
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
 * FUNCTION :  PrintAt
 * ACTION   : Wypisz tekst na AKTUALNYM URZADZENIU LOGICZNYM
 *-----------------------------------------------------------------------------------*/

static void PrintAt(const char* apText, U8 aX, U8 aY)
{
    Font8x8_Print(apText, Screen_GetpLogic(), aX * CHAR_WIDTH, aY * CHAR_HEIGHT);
}

/* ###################################################################################
#  PUBLIC FUNCTIONS
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION : ClearScreen
 * ACTION   :  Wyczysc OBA ekrany
 *-----------------------------------------------------------------------------------*/

void ClearScreen(void)
{
    Screen_Logic_ClearScreen();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawTitle
 * ACTION   : Rysuj tytul na obu buforach
 *-----------------------------------------------------------------------------------*/

void DrawStatus(const char* apText)
{
    U8 lLen;
    U8 lX;

    lLen = String_StrLen(apText);
    lX = (SCREEN_CHARS_X - lLen) / 2;
    PrintAt(apText, lX, STATUS_Y);
}

void DrawTitle(void)
{
    PrintAt(DEBUG, 0, 0);
    PrintAt("================================", 4, 2);
    PrintAt("         H A N G M A N          ", 4, 4);
    PrintAt("         ATARI ST PORT          ", 4, 6);
    PrintAt("================================", 4, 8);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawMenu
 * ACTION   : Rysuj menu z zaznaczona opcja
 *-----------------------------------------------------------------------------------*/

void DrawMenu(U8 aSelectedOption)
{
    U8 i, len;
    char tmpStr[50];
    extern U8 lSelectedLanguage;
    extern U8 lSelectedLevel;

    for (i = 0; i < 4; i++) {
        U8 lY = 12 + (i * 2);

        switch(i) {
            case 0:
                String_StrCpy(tmpStr, GetText(GRP_MENU, i)); /* START GAME */
                break;
            case 1:
                sprintf(tmpStr, "%s: %s", GetText(GRP_MENU, STR_MENU_LEVEL), GetText(GRP_LEVELS, lSelectedLevel)); /* LEVEL */
                break;
            case 2:
                sprintf(tmpStr, "%s (%s)", GetText(GRP_MENU, STR_MENU_LANGUAGE), gDict.mLangs[lSelectedLanguage].mCode); /* LANGUAGE */
                break;
            case 3:
                String_StrCpy(tmpStr, GetText(GRP_MENU, STR_MENU_QUIT)); /* QUIT */
                break;
        }

        if (i == aSelectedOption) {
            PrintAt(">>", 5, lY);
            PrintAt(tmpStr, 8, lY);
            PrintAt("<<", 33, lY);
        }
        else {
            PrintAt("  ", 4, lY);
            PrintAt(tmpStr, 8, lY);
            PrintAt("  ", 33, lY);
        }
    }

    sprintf(tmpStr, "%s %d", GetText(GRP_COMMON, STR_COMMON_DICT_SIZE), GetWordCount());
    len = String_StrLen(tmpStr);
    PrintAt(tmpStr, 20-(len/2), 24);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawAlphabet
 * ACTION   :  Rysuj alfabet z uzytymi literami
 *-----------------------------------------------------------------------------------*/

static void DrawAlphabet(void)
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
            PrintAt("[", lX - 1, lY);
            PrintAt(lStr, lX, lY);
            PrintAt("]", lX + 1, lY);
        }
        else {
            PrintAt(" ", lX - 1, lY);
            PrintAt(lStr, lX, lY);
            PrintAt(" ", lX + 1, lY);
        }
    }
}
/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawAnswer
 * ACTION   : Rysuj haslo do odgadniecia
 *-----------------------------------------------------------------------------------*/

static void DrawAnswer(void)
{
    U8 i;
    U8 lLen;
    U8 lStartX;
    char lStr[2] = { 0, 0 };

    PrintAt(GetText(GRP_COMMON, STR_COMMON_SECRET), 2, ANSWER_Y);

    lLen = 0;
    while (gGame.mAnswer[lLen] != '\0') lLen++;
    
    lStartX = (SCREEN_CHARS_X - (lLen * 2)) / 2;

    for (i = 0; i < lLen; i++) {
        lStr[0] = gGame.mAnswer[i];
        PrintAt(lStr, lStartX + (i * 2), ANSWER_Y + 2);
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawHangman
 * ACTION   :  Rysuj wisielca (tekstowo)
 *-----------------------------------------------------------------------------------*/

static void DrawHangman(void)
{
    U8 lMisses = gGame.mMisses;
    char lNum[4];

    PrintAt(GetText(GRP_COMMON, STR_COMMON_ERRORS), HANGMAN_X, HANGMAN_Y);

    lNum[0] = '0' + (lMisses / 10);
    lNum[1] = '0' + (lMisses % 10);
    lNum[2] = '/';
    lNum[3] = '\0';
    PrintAt(lNum, HANGMAN_X + 8, HANGMAN_Y);
    PrintAt("12", HANGMAN_X + 11, HANGMAN_Y);

    PrintAt(" +---+  ", HANGMAN_X, HANGMAN_Y + 2);
    PrintAt(" |   |  ", HANGMAN_X, HANGMAN_Y + 3);

    if (lMisses >= 1) {
        PrintAt(" |   O  ", HANGMAN_X, HANGMAN_Y + 4);
    } else {
        PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 4);
    }

    if (lMisses >= 6) {
        PrintAt(" |  /|\\ ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 5) {
        PrintAt(" |  /|  ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 4) {
        PrintAt(" |   |  ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 3) {
        PrintAt(" |  \\|  ", HANGMAN_X, HANGMAN_Y + 5);
    } else if (lMisses >= 2) {
        PrintAt(" |  \\   ", HANGMAN_X, HANGMAN_Y + 5);
    } else {
        PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 5);
    }

    if (lMisses >= 7) {
        PrintAt(" |   |  ", HANGMAN_X, HANGMAN_Y + 6);
    } else {
        PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 6);
    }

    if (lMisses >= 12) {
        PrintAt(" |  / \\ ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 11) {
        PrintAt(" |  /   ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 10) {
        PrintAt(" |    \\ ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 9) {
        PrintAt(" |  |   ", HANGMAN_X, HANGMAN_Y + 7);
    } else if (lMisses >= 8) {
        PrintAt(" |    | ", HANGMAN_X, HANGMAN_Y + 7);
    } else {
        PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 7);
    }

    PrintAt(" |      ", HANGMAN_X, HANGMAN_Y + 8);
    PrintAt("===     ", HANGMAN_X, HANGMAN_Y + 9);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawGameContent
 * ACTION   : Rysuje zawartosc ekranu gry (helper)
 *-----------------------------------------------------------------------------------*/

void DrawGameContent(const char* apText)
{
    ClearScreen();
    PrintAt("=== HANGMAN ===", 12, 0);
    DrawAlphabet();
    DrawAnswer();
    DrawHangman();
    DrawStatus(apText);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : DrawGame
 * ACTION   : Rysuj caly ekran gry NA OBU BUFORACH
 *-----------------------------------------------------------------------------------*/

void DrawGame(const char* apText)
{
    DrawGameContent(apText);
    Screen_Update();
    DrawGameContent(apText);
    Screen_Update();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : UpdateCursor
 * ACTION   :  Aktualizuj tylko kursor NA OBU BUFORACH
 *-----------------------------------------------------------------------------------*/

void UpdateCursor(void)
{
    DrawAlphabet();
    Screen_Update();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : ShowMessage
 * ACTION   :  Pokaz wiadomosc NA OBU BUFORACH
 *-----------------------------------------------------------------------------------*/

void ShowMessage(const char* apMessage)
{
    U8 i;
    U8 lLen;
    U8 lX;

    /* Oblicz dlugosc */
    lLen = 0;
    while (apMessage[lLen] != '\0') lLen++;
    lX = (SCREEN_CHARS_X - lLen) / 2;

    for (i = 0; i < 2; i++) {
        PrintAt("                                        ", 0, MESSAGE_Y);
        PrintAt(apMessage, lX, MESSAGE_Y);
        Screen_Update();
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : ShowEndScreen
 * ACTION   : Pokaz ekran konca gry
 *-----------------------------------------------------------------------------------*/

void ShowEndScreen(U8 aWin)
{
    U8 lLen;
    U8 lX;
    char* gTemp;

    if (aWin) {
        gTemp = GetRandomText(GRP_WINS);
        lLen = String_StrLen(gTemp);
        lX = (SCREEN_CHARS_X - lLen) / 2;
        PrintAt("================================", 4, 8);
        PrintAt(gTemp, lX, 10);
        PrintAt("================================", 4, 12);
    }
    else {
        gTemp = GetRandomText(GRP_LOOSES);
        lLen = String_StrLen(gTemp);
        lX = (SCREEN_CHARS_X - lLen) / 2;
        PrintAt("================================", 4, 8);
        PrintAt(gTemp, lX, 10);
        PrintAt("================================", 4, 12);
    }
    /* Oblicz srodek dla hasla */
    lLen = String_StrLen(GetText(GRP_COMMON, STR_COMMON_GUESSED));
    lX = (SCREEN_CHARS_X - lLen) / 2;
    PrintAt(GetText(GRP_COMMON, STR_COMMON_GUESSED), lX, 14);

    lLen = String_StrLen(gGame.mAnswer);
    lX = (SCREEN_CHARS_X - lLen) / 2;
    PrintAt(gGame.mAnswer, lX, 16);

    // DrawHangman();

    PrintAt(GetText(GRP_COMMON, STR_COMMON_PRESS_ANY), 6, 22);
}

/* ################################################################################ */