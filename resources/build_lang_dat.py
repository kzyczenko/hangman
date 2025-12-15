#!/usr/bin/env python3
# -*- coding: utf-8 -*-import struct
import os
import sys
import struct

langcode = sys.argv[1]

# Konfiguracja
INPUT_FILE = f'{langcode}/lang.ini'
OUTPUT_DAT = f'../{langcode}/LANG.DAT'
OUTPUT_HEADER = '../src/lang.h'

def compile_language():
    if not os.path.exists(INPUT_FILE):
        print(f"BŁĄD: Brak pliku {INPUT_FILE}")
        return

    # Struktury danych
    groups = []      # Lista list stringów (każda grupa to lista napisów)
    header_lines = []
    
    header_lines.append("#ifndef LANG_IDS_H")
    header_lines.append("#define LANG_IDS_H")
    header_lines.append("\n/* Grupy komunikatów */")
    
    current_strings = []
    current_section_name = ""
    group_id = 0
    item_idx = 0

    with open(INPUT_FILE, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # --- PARSOWANIE LINIA PO LINII ---
    for line in lines:
        line = line.strip()
        
        # Pomiń puste linie i komentarze
        if not line or line.startswith(';'):
            continue

        # 1. Wykrycie nowej sekcji [NAZWA]
        if line.startswith('[') and line.endswith(']'):
            # Jeśli mieliśmy już otwartą sekcję, zapisujemy ją do listy grup
            if current_section_name != "":
                groups.append(current_strings)
                current_strings = [] # Reset bufora
                item_idx = 0
                group_id += 1

            # Odczytaj nową nazwę sekcji
            section_raw = line[1:-1]
            current_section_name = section_raw.upper()
            
            # Dodaj definicję grupy do .h
            header_lines.append(f"\n/* Sekcja: {section_raw} */")
            header_lines.append(f"#define GRP_{current_section_name} {group_id}")
            continue

        # 2. Wykrycie pary Klucz = Wartość
        if '=' in line:
            # Dzielimy tylko na pierwszym znaku równości
            key, value = line.split('=', 1)
            key = key.strip()
            value = value.strip()
            
            # Dodaj napis do aktualnej listy
            current_strings.append(value)
            
            # Jeśli klucz NIE JEST losowy (_RND), to generujemy dla niego stałą #define
            # (ignorujemy wielkość liter klucza w .ini dla wygody, ale w .h robimy UPPER)
            if key.upper() != "_RND":
                header_lines.append(f"#define STR_{current_section_name}_{key.upper()} {item_idx}")
            
            item_idx += 1

    # WAŻNE: Po zakończeniu pętli trzeba zapisać OSTATNIĄ grupę
    if current_section_name != "":
        groups.append(current_strings)
        group_id += 1

    header_lines.append(f"\n#define NUM_GROUPS {group_id}")
    header_lines.append("\n#endif")

    # --- ZAPIS PLIKÓW ---

    # 1. Zapis pliku nagłówkowego .H
    with open(OUTPUT_HEADER, 'w', encoding='utf-8') as f:
        f.write('\n'.join(header_lines))
    print(f"Wygenerowano {OUTPUT_HEADER}")

    # 2. Zapis pliku binarnego .DAT (Format "Atari Friendly")
    with open(OUTPUT_DAT, 'wb') as f:
        # Nagłówek: Ilość grup [2 bajty]
        f.write(struct.pack('>H', len(groups)))
        
        # Tabela liczników (ile stringów w każdej grupie) [2 bajty * ilość grup]
        for grp in groups:
            f.write(struct.pack('>H', len(grp)))
            
        # Dane stringów (ISO-8859-2 + NULL)
        for grp in groups:
            for s in grp:
                try:
                    # Kodowanie do polskiego standardu Atari (lub Windows-1250 / ISO-8859-2)
                    data = s.encode('iso-8859-2') + b'\x00'
                    f.write(data)
                except UnicodeEncodeError:
                    print(f"OSTRZEŻENIE: Nie można zakodować '{s}'. Zastąpiono znakiem '?'.")
                    safe_s = s.encode('ascii', errors='replace') + b'\x00'
                    f.write(safe_s)

    print(f"Wygenerowano {OUTPUT_DAT} ({len(groups)} grup).")

if __name__ == "__main__":
    compile_language()