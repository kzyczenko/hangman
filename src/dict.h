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

#define LANG_INI_FILE       "LANG.INI"
#define LANG_MAP_FILE       "LANG.MAP"
#define LANG_DAT_FILE       "LANG.DAT"

#define MSG_MAX_STRINGS        10
#define MSG_MAX_STRING_LEN     40

#define DICT_MAX_LANGS          2
#define DICT_MAX_ALPHABET_LEN   64
#define DICT_BLOCK_SIZE_SHORT   0x1000  /* 4KB - słowa 3-7 znaków */
#define DICT_BLOCK_SIZE_LONG    0x2000  /* 8KB - słowa 8-13 znaków */
#define DICT_WORD_SIZE_SHORT    8
#define DICT_WORD_SIZE_LONG     16

/* Poziomy trudności (bitmaski) */
#define LVL1   0x01
#define LVL2   0x02
#define LVL3   0x04
#define LVL4   0x08
#define LVLALL 0x3F

/* Długość słów (bitmaski) */
#define TO7    0x01    /* słowa 3-7 znaków */
#define TO13   0x02    /* słowa 8-13 znaków */

/* ###################################################################################
#  STRUCTS
################################################################################### */

/* Informacje o dostępnym języku */
typedef struct sLangInfo {
    char mCode[3];              /* Kod języka:  "PL", "EN", etc. */
    U8   mBlockCount[2][4];     /* Liczba bloków [TO7/TO13][LVL1-4] */
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
    U8*         mpWordBlock;        /* Załadowany blok */
    U16         mWordBlockSize;     /* Rozmiar bloku */
    U8          mWordSize;          /* Rozmiar pojedynczego słowa (8 lub 16) */
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
U8 LoadRandomBlock(void);
void GetRandomWord(char* apBuffer);
void GetWordAt(U16 aIndex, char* apBuffer);

/* Pobieranie tekstów */
const char* GetAlphabet(void);
U8   GetAlphabetLen(void);

char* GetText(U16 aGroupId, U16 aStringIdx);
char* GetRandomText(U16 aGroupId);

/* ################################################################################ */

#endif /* INCLUDED_DICT_H */