#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import struct
import os
import sys

langcode = sys.argv[1]


# --- KONFIGURACJA ---
INPUT_FILE = f'{langcode}/dict.txt'  # Musi być w UTF-8
MIN_LEN = 3
MAX_LEN = 13                # Żeby mieściły się na ekranie ST

# Wagi liter (im wyższa, tym rzadsza/trudniejsza)
LETTER_WEIGHTS = {
    # Pospolite (1 pkt)
    'A':1, 'I':1, 'O':1, 'E':1, 'Z':1, 'N':1, 'R':1, 'W':1, 'S':1, 'C':1, 'Y':1, 'K':1,
    # Średnie (2 pkt) - domyślne dla reszty spółgłosek
    'D':2, 'P':2, 'M':2, 'U':2, 'J':2, 'L':2, 'T':2, 'B':2, 'G':2,
    # Trudne / Polskie (4-5 pkt)
    'H':3, 'Ł':3, 'F':4, 'Ó':4, 'Ą':4, 'Ę':4, 'Ś':4, 'Ć':4, 'Ń':4, 'Ź':4, 'Ż':4, 'V':5
}

def calculate_difficulty(word):
    """Zwraca wynik trudności słowa (avg_score)."""
    score = 0
    clean_word = word.upper().strip()
    if len(clean_word) == 0: return 0
    
    for char in clean_word:
        score += LETTER_WEIGHTS.get(char, 2) # Domyślnie 2
    
    return score / len(clean_word)

def classify_word(word):
    """Przydziela słowo do poziomu 1, 2 lub 3."""
    word = word.upper().strip()
    length = len(word)
    avg = calculate_difficulty(word)

    # POZIOM 3 (HARD)
    # Bardzo rzadkie litery LUB bardzo długie LUB bardzo krótkie (3 litery to często pułapka)
    if avg >= 2.2 or length >= 11 or length == 3:
        return 3
        
    # POZIOM 1 (EASY)
    # Standardowa długość i bardzo popularne litery
    if avg <= 1.5 and (5 <= length <= 8):
        return 1
        
    # POZIOM 2 (MEDIUM)
    # Wszystko pomiędzy
    return 2

def generate_binary_files():
    buckets = {1: [], 2: [], 3: []}
    
    if not os.path.exists(INPUT_FILE):
        print(f"BŁĄD: Nie znaleziono pliku {INPUT_FILE}")
        return

    print(f"Wczytywanie {INPUT_FILE}...")
    
    with open(INPUT_FILE, 'r', encoding='utf-8') as f:
        for line in f:
            word = line.strip().upper()
            
            # Filtrowanie długości
            if MIN_LEN <= len(word) <= MAX_LEN:
                # Klasyfikacja
                level = classify_word(word)
                
                # Próba kodowania do standardu Atari (ISO-8859-2)
                try:
                    # Dodajemy \0 na końcu (ważne dla C!)
                    encoded_word = word.encode('iso-8859-2') + b'\x00'
                    buckets[level].append(encoded_word)
                except UnicodeEncodeError:
                    # Ignorujemy słowa z dziwnymi znakami (spoza PL/EN)
                    continue

    # Zapis do plików .DAT
    for lvl in [1, 2, 3]:
        words_list = buckets[lvl]
        count = len(words_list)
        filename = f"../{langcode}/LEVEL{lvl}.DAT"
        
        with open(filename, 'wb') as out:
            # 1. NAGŁÓWEK: Liczba słów (2 bajty, Big Endian)
            # '>' = Big Endian (Motorola), 'H' = unsigned short
            out.write(struct.pack('>H', count))
            
            # 2. DANE: Ciąg stringów oddzielonych zerami
            for w_bytes in words_list:
                out.write(w_bytes)
                
        # Statystyki
        file_size = os.path.getsize(filename)
        print(f"--- {filename} ---")
        print(f"  Słów: {count}")
        print(f"  Rozmiar pliku: {file_size} bajtów ({file_size/1024:.1f} KB)")
        if count > 0:
            print(f"  Przykłady: {words_list[0][:-1].decode('iso-8859-2')}, {words_list[-1][:-1].decode('iso-8859-2')}")

if __name__ == "__main__":
    generate_binary_files()