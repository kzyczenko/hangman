/*************************************************************************************
 * Hangman ST - Dictionary Implementation
 *************************************************************************************
 */

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include "dict.h"
#include "ui.h"
#include "lang.h"
#include <godlib/file/file.h>
#include <godlib/memory/memory.h>
#include <godlib/string/string.h>
#include <godlib/random/random.h>
#include <godlib/vbl/vbl.h>
#include <string.h>

/* ###################################################################################
#  DATA
################################################################################### */

sDictionary gDict;
U8 lTotalBlocks = 0;

sTextGroup *gGroupsTable = NULL; // Tablica struktur grup
char *gTextBlob = NULL;         // Surowe dane
U16 lNumGroups = 0;

char DEBUG[64];

/* ###################################################################################
#  HELPERS
################################################################################### */


/* ###################################################################################
#  PRIVATE FUNCTIONS
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION :  RandomRange
 * ACTION   :  Losowa liczba z zakresu [0, aMax)
 *-----------------------------------------------------------------------------------*/

static U16 RandomRange(U16 aMax)
{
    if (aMax == 0) return 0;
    return Random_GetClamped(aMax);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : LoadLangMap
 * ACTION   :  Ładuje mapę języków (oryginalny format Atari 8-bit)
 *-----------------------------------------------------------------------------------*/

static void LoadLangMap(void)
{
    U8* lpData;
    U8* lpPtr;
    U8 i;
    
    lpData = File_Load(LANG_MAP_FILE);
    
    if (! lpData) {
        return;
    }
    
    lpPtr = lpData;
    
    /* Bajt 0: Liczba języków */
    gDict.mLangCount = *lpPtr++;
    if (gDict.mLangCount > DICT_MAX_LANGS) {
        gDict.mLangCount = DICT_MAX_LANGS;
    }
    
    /* Bajty 1-(langCount*2): Kody języków */
    for (i = 0; i < gDict.mLangCount; i++) {
        gDict.mLangs[i].mCode[0] = *lpPtr++;
        gDict.mLangs[i].mCode[1] = *lpPtr++;
        gDict.mLangs[i].mCode[2] = '\0';
    }
        
    File_UnLoad(lpData);
}

static void SetAlphabet(const char *alphabet)
{
    U8 len = String_StrLen(alphabet);
    if (len > DICT_MAX_ALPHABET_LEN) {
        len = DICT_MAX_ALPHABET_LEN;
    }
    Memory_Copy(len, alphabet, gDict.mAlphabet);
    gDict.mAlphabet[len] = '\0';
    gDict.mAlphabetLen = (U8)len;
}

/* --- Parser pliku --- */
/* Zwraca 1 = OK, 0 = błąd (np. nie otwarto pliku) */
static S16 LoadLangDat()
{
    sFileHandle lHandle;
    U32 lFilesize;
    U16 i, j;
    char lFilename[16];
    char *ptr;

    // Sprzątanie poprzedniego języka
    if (gGroupsTable) { 
        // Zwalniamy tablice wskaźników dla każdej grupy
        for(i=0; i<lNumGroups; i++) {
            if(gGroupsTable[i].mStrings) mMEMFREE(gGroupsTable[i].mStrings);
        }
        mMEMFREE(gGroupsTable); 
        gGroupsTable = NULL; 
    }
    if (gTextBlob) { mMEMFREE(gTextBlob); gTextBlob = NULL; }

    sprintf(lFilename, "%s\\%.24s", gDict.mLangs[gApp.mCurrentLang].mCode, LANG_DAT_FILE);    

    lHandle = File_Open(lFilename);
    if (lHandle<0) return 0;

    // 1. Czytamy ilość grup
    File_Read(lHandle, 2, &lNumGroups);
    // 2. Alokujemy tabelę grup
    gGroupsTable = (sTextGroup*)mMEMALLOC(lNumGroups * sizeof(sTextGroup));
    
    // 3. Wczytujemy liczniki dla każdej grupy
    for(i=0; i<lNumGroups; i++) {
        U16 lCount;
        File_Read(lHandle, 2, &lCount);
        gGroupsTable[i].mCount = lCount;
        // Alokujemy od razu tablicę wskaźników dla tej grupy
        gGroupsTable[i].mStrings = (char**)mMEMALLOC(lCount * sizeof(char*));
    }

    // 4. Reszta pliku to tekst - wczytujemy do bloba
    U32 cur_pos = File_SeekFromCurrent(lHandle, 0);
    File_SeekFromEnd(lHandle, 0);
    lFilesize = File_SeekFromCurrent(lHandle, 0) - cur_pos;
    File_SeekFromStart(lHandle, cur_pos);

    gTextBlob = (char*)mMEMALLOC(lFilesize);
    File_Read(lHandle, lFilesize, gTextBlob);
    File_Close(lHandle);

    // 5. POINTER FIXUP (Wiązanie wskaźników)
    ptr = gTextBlob;
    for(i=0; i<lNumGroups; i++) {
        for(j=0; j<gGroupsTable[i].mCount; j++) {
            gGroupsTable[i].mStrings[j] = ptr; // Przypisz adres
            // Przesuń ptr za koniec stringa
            while(*ptr++); 
        }
    }

    SetAlphabet(GetText(GRP_ALPHABETS, STR_ALPHABETS_UP));
    return 1;
}

/* ###################################################################################
#  PUBLIC FUNCTIONS
################################################################################### */

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Dict_Init
 * ACTION   : Inicjalizacja słownika
 *-----------------------------------------------------------------------------------*/

void Dict_Init(void)
{
    U8 i;
        
    /* Wyzeruj strukturę ręcznie */
    gDict.mLangCount = 0;
    gDict.mAlphabetLen = 0;
    gDict.mAlphabet[0] = '\0';
    gDict.mpWordBlock = 0;
    gDict.mpWordIndex = 0;
    gDict.mWordBlockSize = 0;
    gDict.mWordCount = 0;
    
    for (i = 0; i < DICT_MAX_LANGS; i++) {
        gDict.mLangs[i].mCode[0] = '\0';
        gDict.mLangs[i].mCode[1] = '\0';
        gDict.mLangs[i].mCode[2] = '\0';
    }
    
    /* Załaduj mapę języków */
    LoadLangMap();
    
    /* Załaduj pierwszy język */
    SetLang(0);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : Dict_DeInit
 * ACTION   :  Deinicjalizacja słownika
 *-----------------------------------------------------------------------------------*/

void Dict_DeInit(void)
{
    if (gDict.mpWordBlock) {
        File_UnLoad(gDict.mpWordBlock);
        gDict.mpWordBlock = 0;
    }
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetLangCount
 * ACTION   :  Zwraca liczbę dostępnych języków
 *-----------------------------------------------------------------------------------*/

U8 GetLangCount(void)
{
    return gDict.mLangCount;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetLangCode
 * ACTION   :  Zwraca kod języka
 *-----------------------------------------------------------------------------------*/

const char* GetLangCode(U8 aIndex)
{
    if (aIndex >= gDict.mLangCount) {
        return "??";
    }
    return gDict.mLangs[aIndex].mCode;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : SetLang
 * ACTION   :  Ustawia aktywny język
 *-----------------------------------------------------------------------------------*/

void SetLang(U8 aIndex)
{
    if (aIndex >= gDict.mLangCount) {
        aIndex = 0;
    }
    
    gApp.mCurrentLang = aIndex;
    
    /* Zwolnij stary blok słów */
    if (gDict.mpWordBlock) {
        File_UnLoad(gDict.mpWordBlock);
        gDict.mpWordBlock = 0;
    }
    
    /* Załaduj dane językowe */
    LoadLangDat();
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetCurrentLang
 * ACTION   :  Zwraca indeks aktualnego języka
 *-----------------------------------------------------------------------------------*/

U8 GetCurrentLang(void)
{
    return gApp.mCurrentLang;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : SetLevel
 * ACTION   : Ustawia poziom trudności
 *-----------------------------------------------------------------------------------*/

void SetLevel(U8 aLevelMask)
{
    gApp.mCurrentLevel = aLevelMask;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetLevel
 * ACTION   : Zwraca maskę poziomu
 *-----------------------------------------------------------------------------------*/

U8 GetLevel(void)
{
    return gApp.mCurrentLevel;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetWordCount
 * ACTION   :  Zwraca liczbę dostępnych słów dla aktualnych ustawień
 *-----------------------------------------------------------------------------------*/

U16 GetWordCount(void)
{    
    return gDict.mWordCount;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : LoadDictionary
 * ACTION   : Ładuje losowy blok słów zgodny z aktualnymi ustawieniami
 *-----------------------------------------------------------------------------------*/

U8 LoadDictionary(void) {
    char gFilename[16];
    sFileHandle lHandle;
    U32 lDataLen;

    sprintf(gFilename, "%s\\LEVEL%d.DAT", gDict.mLangs[gApp.mCurrentLang].mCode, gApp.mCurrentLevel+1);
    lHandle = File_Open(gFilename);
    if(lHandle < 0) return 0;

    lDataLen = File_GetSize(gFilename) - sizeof(U16);
 
    File_Read(lHandle, sizeof(U16), &gDict.mWordCount);

    gDict.mpWordBlock = mMEMALLOC(lDataLen);
    File_Read(lHandle, lDataLen, gDict.mpWordBlock);
    File_Close(lHandle);

    gDict.mpWordIndex = mMEMALLOC(gDict.mWordCount * sizeof(char*));
    
    char *ptr = gDict.mpWordBlock;
    for(int i=0; i<gDict.mWordCount; i++) {
        gDict.mpWordIndex[i] = ptr;
        while(*ptr++); 
    }
    
    return 1;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetRandomWord
 * ACTION   :  Pobiera losowe słowo
 *-----------------------------------------------------------------------------------*/

void GetRandomWord(char* apBuffer)
{
    U16 lIndex;

    /* Załaduj blok jeśli nie ma */
    if (!gDict.mpWordBlock) {
        LoadDictionary();
    }

    if (!gDict.mpWordBlock || gDict.mWordCount == 0) {
        /* Fallback - wbudowane słowo */
    String_StrCpy(apBuffer, "HANGMAN");
        return;
    }

    lIndex = RandomRange(gDict.mWordCount);
    String_StrCpy(apBuffer, gDict.mpWordIndex[lIndex]);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetAlphabet
 * ACTION   :  Zwraca alfabet
 *-----------------------------------------------------------------------------------*/

const char* GetAlphabet(void)
{
    return gDict.mAlphabet;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetAlphabetLen
 * ACTION   : Zwraca długość alfabetu
 *-----------------------------------------------------------------------------------*/

U8 GetAlphabetLen(void)
{
    return gDict.mAlphabetLen;
}

/* Funkcja pobierająca konkretny tekst */
char* GetText(U16 aGroupId, U16 aStringIdx) {
    if (aGroupId >= lNumGroups) return "ERR_GRP";
    if (aStringIdx >= gGroupsTable[aGroupId].mCount) return "ERR_IDX";
    
    return gGroupsTable[aGroupId].mStrings[aStringIdx];
}

/* Funkcja losująca tekst z grupy (dla 'Nie ta', 'Pudło' etc.) */
char* GetRandomText(U16 aGroupId) {
    if (aGroupId >= lNumGroups) return "ERR_GRP";
    int count = gGroupsTable[aGroupId].mCount;
    if (count == 0) return "";
    
    return gGroupsTable[aGroupId].mStrings[Random_GetClamped(count-1)];
}
/* ################################################################################ */