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

const U8 lvlTab[4] = { LVL1, LVL2, LVL3, LVL4 };
const U8 toTab[2]  = { TO7, TO13 };
static U8 currentWordLen;
/* Bufory na ścieżki plików */
char gPathBuffer[32];
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
 * FUNCTION : BuildLangPath
 * ACTION   : Buduje ścieżkę do pliku językowego
 *-----------------------------------------------------------------------------------*/

static void BuildLangPath(const char* apFilename)
{
    /* Format: "XX\FILENAME" gdzie XX to kod języka. */
    if (apFilename == NULL) apFilename = "";

    sprintf(gPathBuffer, "%s\\%.24s",
             gDict.mLangs[gApp.mCurrentLang].mCode,
             apFilename);
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : BuildBlockPath
 * ACTION   :  Buduje ścieżkę do bloku słów
 *-----------------------------------------------------------------------------------*/

static void BuildBlockPath(U8 aLenType, U8 aLevel, U8 aBlockNum)
{
    /* Format: "XX\Bty.DAT" */
    sprintf(gPathBuffer, "%s\\B%d%d%d.DAT",
        gDict.mLangs[gApp.mCurrentLang].mCode,
        aLenType,
        lvlTab[aLevel & 3],
        aBlockNum
    );
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : LoadLangMap
 * ACTION   :  Ładuje mapę języków (oryginalny format Atari 8-bit)
 *-----------------------------------------------------------------------------------*/

static void LoadLangMap(void)
{
    U8* lpData;
    U8* lpPtr;
    U8 i, j;
    
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
    
    /* Po kodach:  tablice bloków (8 bajtów na język) */
    for (i = 0; i < gDict.mLangCount; i++) {
        /* TO7 (słowa 3-7 znaków) */
        for (j = 0; j < 4; j++) {
            gDict.mLangs[i].mBlockCount[0][j] = *lpPtr++;
        }
        /* TO13 (słowa 8-13 znaków) */
        for (j = 0; j < 4; j++) {
            gDict.mLangs[i].mBlockCount[1][j] = *lpPtr++;
        }
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
    char *ptr;

    BuildLangPath(LANG_DAT_FILE);
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

    
    lHandle = File_Open(gPathBuffer);
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
    U8 i, j, k;
        
    /* Wyzeruj strukturę ręcznie */
    gDict.mLangCount = 0;
    gDict.mAlphabetLen = 0;
    gDict.mAlphabet[0] = '\0';
    gDict.mpWordBlock = 0;
    gDict.mWordBlockSize = 0;
    gDict.mWordSize = 0;
    gDict.mWordCount = 0;
    
    for (i = 0; i < DICT_MAX_LANGS; i++) {
        gDict.mLangs[i].mCode[0] = '\0';
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 4; k++) {
                gDict.mLangs[i].mBlockCount[j][k] = 0;
            }
        }
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
    U16 lCount = 0;
    U8 lLevel = gApp.mCurrentLevel;
    sLangInfo* lpLang = &gDict.mLangs[gApp.mCurrentLang];
    U8 i;
    
    /* Słowa krótkie (TO7) */
    if (lLevel>>4 & TO7) {
        for (i = 0; i < 4; i++) {
            if (lLevel & (1 << i)) {
                lCount += lpLang->mBlockCount[0][i] << 9;
            }
        }
    }
    
    /* Słowa długie (TO13) */
    if (lLevel>>4 & TO13) {
        for (i = 0; i < 4; i++) {
            if (lLevel & (1 << i)) {
                lCount += lpLang->mBlockCount[1][i] << 9;
            }
        }
    }
    
    return lCount;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : LoadRandomBlock
 * ACTION   : Ładuje losowy blok słów zgodny z aktualnymi ustawieniami
 *-----------------------------------------------------------------------------------*/

U8 LoadRandomBlock(void)
{
    U8 lLevel = gApp.mCurrentLevel;
    sLangInfo* lpLang = &gDict.mLangs[gApp.mCurrentLang];
    U8 lLvl = 0;
    U8 lBlockNum = 0;
    U8 lSelectedBlock;
    U8 i;
    U8 lCount;
    U8 lFound = 0;
    
    /* Zwolnij stary blok */
    if (gDict.mpWordBlock) {
        File_UnLoad(gDict.mpWordBlock);
        gDict.mpWordBlock = 0;
    }
    
    /* Policz dostępne bloki */
    if (lLevel>>4 & TO7) {
        for (i = 0; i < 4; i++) {
            if (lLevel & (1 << i)) {
                lTotalBlocks += lpLang->mBlockCount[0][i];
            }
        }
    }
    
    if (lLevel>>4 & TO13) {
        for (i = 0; i < 4; i++) {
            if (lLevel & (1 << i)) {
                lTotalBlocks += lpLang->mBlockCount[1][i];
            }
        }
    }
    
    if (lTotalBlocks == 0) {
        return 0;
    }
    
    /* Wybierz losowy blok */
    lSelectedBlock = (U8)RandomRange(lTotalBlocks);
    
    /* Znajdź wybrany blok - TO7 */
    if ((lLevel>>4 & TO7) && !lFound) {
        for (i = 0; i < 4 && !lFound; i++) {
            if (lLevel & (1 << i)) {
                lCount = lpLang->mBlockCount[0][i];
                if (lSelectedBlock < lCount) {
                    currentWordLen = TO7;
                    lLvl = i;
                    lBlockNum = lSelectedBlock;
                    lFound = 1;
                }
                else {
                    lSelectedBlock -= lCount;
                }
            }
        }
    }
    
    /* Znajdź wybrany blok - TO13 */
    if ((lLevel>>4 & TO13) && !lFound) {
        for (i = 0; i < 4 && !lFound; i++) {
            if (lLevel & (1 << i)) {
                lCount = lpLang->mBlockCount[1][i];
                if (lSelectedBlock < lCount) {
                    currentWordLen = TO13;
                    lLvl = i;
                    lBlockNum = lSelectedBlock;
                    lFound = 1;
                }
                else {
                    lSelectedBlock -= lCount;
                }
            }
        }
    }
    
    /* Zbuduj ścieżkę i załaduj */
    BuildBlockPath(currentWordLen, lLvl, lBlockNum);
    gDict.mpWordBlock = File_Load(gPathBuffer);
    
    if (gDict.mpWordBlock) {
        if (currentWordLen == TO7) {
            gDict.mWordBlockSize = DICT_BLOCK_SIZE_SHORT;
            gDict.mWordSize = DICT_WORD_SIZE_SHORT;
        } else {
            gDict.mWordBlockSize = DICT_BLOCK_SIZE_LONG;
            gDict.mWordSize = DICT_WORD_SIZE_LONG;
        }
        gDict.mWordCount = gDict.mWordBlockSize / gDict.mWordSize;
    }
    return lBlockNum;
}

/*-----------------------------------------------------------------------------------*
 * FUNCTION : GetWordAt
 * ACTION   : Pobiera słowo z aktualnego bloku
 *-----------------------------------------------------------------------------------*/

void GetWordAt(U16 aIndex, char* apBuffer)
{
    U8* lpWord;
    U8 lLen;
    U8 i;

    if (! gDict.mpWordBlock || aIndex >= gDict.mWordCount) {
        apBuffer[0] = '\0';
        return;
    }

    lpWord = gDict.mpWordBlock + (aIndex * gDict.mWordSize);
    lLen = lpWord[0];

    for (i = 0; i < lLen; i++) {
        apBuffer[i] = lpWord[1 + i];
    }
    apBuffer[lLen] = '\0';
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
        LoadRandomBlock();
    }

    if (!gDict.mpWordBlock || gDict.mWordCount == 0) {
        /* Fallback - wbudowane słowo */
    String_StrCpy(apBuffer, "HANGMAN");
        return;
    }

    lIndex = RandomRange(gDict.mWordCount);
    GetWordAt(lIndex, apBuffer);
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