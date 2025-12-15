/*************************************************************************************
 * Hangman ST - Dictionary Header
 *************************************************************************************
 */

#ifndef INCLUDED_DICT_H
#define INCLUDED_DICT_H

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include <godlib/base/base.h>

/* ###################################################################################
#  DEFINES
################################################################################### */

#define LANG_MAP_FILE       "LANG.MAP"
#define LANG_DAT_FILE       "LANG.DAT"

#define DICT_MAX_LANGS          2
#define DICT_MAX_ALPHABET_LEN   64

/* ###################################################################################
#  STRUCTS
################################################################################### */

/* Informacje o dostępnym języku */
typedef struct sLangInfo {
    char mCode[3];              /* Kod języka:  "PL", "EN", etc. */
} sLangInfo;

/* Załadowany słownik */
typedef struct sDictionary {
    /* Języki */
    U8          mLangCount;
    sLangInfo   mLangs[DICT_MAX_LANGS];
    
    /* Alfabet (duże litery) */
    char        mAlphabet[DICT_MAX_ALPHABET_LEN + 1];
    U8          mAlphabetLen;

    /* Aktualny blok słów */
    char*       mpWordBlock;        /* Załadowany blok */
    char**      mpWordIndex;        /* Indeksy słów w bloku */
    U16         mWordBlockSize;     /* Rozmiar bloku */
    U16         mWordCount;         /* Liczba słów w bloku */
} sDictionary;

typedef struct sTextGroup {
    U16         mCount; // Ile stringów w grupie
    char        **mStrings;       // Tablica wskaźników do stringów
} sTextGroup;

typedef struct sTextBlob {
    U16         mSize; // Ile stringów w grupie
    U8          mData[1];       // Tablica wskaźników do stringów
} sTextBlob;

/* ###################################################################################
#  GLOBALS
################################################################################### */

extern sTextGroup *gGroupsTable;
extern char *gTextBlob;
extern sDictionary gDict;
extern char gPathBuffer[32];
extern U8 lTotalBlocks;
extern const U8 lvlTab[4];
extern const U8 toTab[2];
extern char DEBUG[64];

/* ###################################################################################
#  PROTOTYPES
################################################################################### */

/* Inicjalizacja/deinicjalizacja */
void Dict_Init(void);
void Dict_DeInit(void);

/* Zarządzanie językami */
U8   GetLangCount(void);
const char* GetLangCode(U8 aIndex);
void SetLang(U8 aIndex);
U8   GetCurrentLang(void);

/* Zarządzanie poziomami */
void SetLevel(U8 aLevelMask);
U8   GetLevel(void);
U16  GetWordCount(void);  /* Liczba dostępnych słów dla aktualnych ustawień */

/* Pobieranie słów */
U8 LoadDictionary(void);
void GetRandomWord(char* apBuffer);
void GetWordAt(U16 aIndex, char* apBuffer);

/* Pobieranie tekstów */
const char* GetAlphabet(void);
U8   GetAlphabetLen(void);

char* GetText(U16 aGroupId, U16 aStringIdx);
char* GetRandomText(U16 aGroupId);

/* ################################################################################ */

#endif /* INCLUDED_DICT_H */