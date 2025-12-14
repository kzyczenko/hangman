/*************************************************************************************
 * Hangman ST - Input Handling
 *************************************************************************************
 */

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include "input.h"
#include <godlib/ikbd/ikbd.h>
#include <godlib/vbl/vbl.h>

/* ###################################################################################
#  DEFINES
################################################################################### */

#define INPUT_DELAY_FRAMES  8

/* Bity pakietu joysticka */
#define JOY_UP      0x01
#define JOY_DOWN    0x02
#define JOY_LEFT    0x04
#define JOY_RIGHT   0x08
#define JOY_FIRE    0x80

/* ###################################################################################
#  DATA
################################################################################### */

static U8 gInputDelay = 0;

/* Mapowanie scancodes na litery */
static const char gScanToChar[] = {
    0,   0,   '1', '2', '3', '4', '5', '6',  /* 0x00-0x07 */
    '7', '8', '9', '0', '-', '=', 0,   0,    /* 0x08-0x0F */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',  /* 0x10-0x17 */
    'O', 'P', '[', ']', 0,   0,   'A', 'S',  /* 0x18-0x1F */
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',  /* 0x20-0x27 */
    '\'', '`', 0,  '\\', 'Z', 'X', 'C', 'V', /* 0x28-0x2F */
    'B', 'N', 'M', ',', '.', '/', 0,   '*',  /* 0x30-0x37 */
};

/* ###################################################################################
#  CODE
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION :  Input_Init
 * ACTION   : Inicjalizacja wejscia
 *-----------------------------------------------------------------------------------*/

void Input_Init(void)
{
    gInputDelay = 0;
    IKBD_EnableJoysticks();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Input_Update
 * ACTION   : Aktualizacja stanu wejscia (wywolywana co klatke)
 *-----------------------------------------------------------------------------------*/

void Input_Update(void)
{
    if (gInputDelay > 0) {
        gInputDelay--;
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Input_GetMenu
 * ACTION   : Pobierz wejscie dla menu
 *-----------------------------------------------------------------------------------*/

U8 Input_GetMenu(void)
{
    U8 lKey;
    U8 lJoy;

    if (gInputDelay > 0) {
        return eINPUT_NONE;
    }

    /* Sprawdz klawiature */
    while (IKBD_GetKbdBytesWaiting()) {
        lKey = IKBD_PopKbdByte();

        /* Ignoruj key release (bit 7 ustawiony) */
        if (lKey & 0x80) {
            continue;
        }

        gInputDelay = INPUT_DELAY_FRAMES;

        if (lKey == eIKBDSCAN_UPARROW) {
            return eINPUT_UP;
        }
        if (lKey == eIKBDSCAN_DOWNARROW) {
            return eINPUT_DOWN;
        }
        if (lKey == eIKBDSCAN_LEFTARROW) {
            return eINPUT_LEFT;
        }
        if (lKey == eIKBDSCAN_RIGHTARROW) {
            return eINPUT_RIGHT;
        }
        if (lKey == eIKBDSCAN_RETURN || lKey == eIKBDSCAN_SPACE) {
            return eINPUT_FIRE;
        }
        if (lKey == eIKBDSCAN_ESC) {
            return eINPUT_QUIT;
        }
    }

    /* Sprawdz joystick 1 */
    lJoy = IKBD_GetJoy1Packet();
    
    if (lJoy & JOY_FIRE) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_FIRE;
    }
    
    if (lJoy & JOY_UP) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_UP;
    }
    if (lJoy & JOY_DOWN) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_DOWN;
    }
    if (lJoy & JOY_LEFT) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_LEFT;
    }
    if (lJoy & JOY_RIGHT) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_RIGHT;
    }

    return eINPUT_NONE;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION :  Input_GetGame
 * ACTION   : Pobierz wejscie dla gry (z obsluga liter)
 *-----------------------------------------------------------------------------------*/

U8 Input_GetGame(void)
{
    U8 lKey;
    U8 lJoy;

    if (gInputDelay > 0) {
        return eINPUT_NONE;
    }

    /* Sprawdz klawiature */
    while (IKBD_GetKbdBytesWaiting()) {
        lKey = IKBD_PopKbdByte();

        /* Ignoruj key release (bit 7 ustawiony) */
        if (lKey & 0x80) {
            continue;
        }

        gInputDelay = INPUT_DELAY_FRAMES;

        /* Klawisze specjalne */
        if (lKey == eIKBDSCAN_UPARROW) {
            return eINPUT_UP;
        }
        if (lKey == eIKBDSCAN_DOWNARROW) {
            return eINPUT_DOWN;
        }
        if (lKey == eIKBDSCAN_LEFTARROW) {
            return eINPUT_LEFT;
        }
        if (lKey == eIKBDSCAN_RIGHTARROW) {
            return eINPUT_RIGHT;
        }
        if (lKey == eIKBDSCAN_RETURN || lKey == eIKBDSCAN_SPACE) {
            return eINPUT_FIRE;
        }
        if (lKey == eIKBDSCAN_ESC) {
            return eINPUT_QUIT;
        }

        /* Litery - konwersja scancode na ASCII */
        if (lKey < sizeof(gScanToChar)) {
            char lChar = gScanToChar[lKey];
            if (lChar >= 'A' && lChar <= 'Z') {
                return (U8)lChar;
            }
        }
    }

    /* Sprawdz joystick 1 */
    lJoy = IKBD_GetJoy1Packet();
    
    if (lJoy & JOY_FIRE) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_FIRE;
    }
    
    if (lJoy & JOY_UP) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_UP;
    }
    if (lJoy & JOY_DOWN) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_DOWN;
    }
    if (lJoy & JOY_LEFT) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_LEFT;
    }
    if (lJoy & JOY_RIGHT) {
        gInputDelay = INPUT_DELAY_FRAMES;
        return eINPUT_RIGHT;
    }

    return eINPUT_NONE;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Input_WaitForAny
 * ACTION   : Czekaj na dowolny klawisz/przycisk
 *-----------------------------------------------------------------------------------*/

void Input_WaitForAny(void)
{
    U8 lJoy;
    
    /* Wyczysc bufor */
    IKBD_ClearKeyPressedFlag();
    /* Czekaj na wcisniecie */
    while (1) {
        IKBD_Update();

        if (IKBD_GetKeyPressedFlag()) {
            while (IKBD_GetKbdBytesWaiting()) {
                IKBD_PopKbdByte();
            }
            IKBD_ClearKeyPressedFlag();
            break;
        }

        lJoy = IKBD_GetJoy1Packet();
        if (lJoy & JOY_FIRE) {
            break;
        }

        Vbl_WaitVbl();
    }

    /* Czekaj na puszczenie fire */
    while (1) {
        IKBD_Update();
        lJoy = IKBD_GetJoy1Packet();
        if (!(lJoy & JOY_FIRE)) {
            break;
        }
        Vbl_WaitVbl();
    }
}

/* ################################################################################ */