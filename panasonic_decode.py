#!/usr/bin/env python3

import sys
from typing import TextIO

def is_short(x: int) -> bool:
    return abs(0x10 - x) < 2
def is_long(x: int) -> bool:
    return abs(0x30 - x) < 2

def parse(f: TextIO) -> None:
    for line in f:
        try:
            data = [int(h, 16) for h in line.split(' ')]
        except ValueError:
            print(line, end='')
            continue

        byte = 0x00
        bits = 0
        data = data[6:] # Remove header and lead in burst
        for i in range(0, len(data), 2):
            pair = data[i], data[i+1]
            if is_short(pair[0]) and is_short(pair[1]):
                byte = (byte >> 1)
                bits += 1
            elif is_short(pair[0]) and is_long(pair[1]):
                byte = (byte >> 1) | 0x80
                bits += 1
            else:
                break
            if bits == 8:
                print(f"{byte:02X}", end=' ')
                bits = 0
                byte = 0x00
        print()

with open(sys.argv[1], 'r') as f:
    parse(f)
