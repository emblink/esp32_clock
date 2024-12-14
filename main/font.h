#pragma once
#include <stdint.h>

#define FONT_SYMBOL_SIZE_IN_BYTES 8

typedef enum SymbolShift {
    FONT_SYMBOL_LEFT_SHIFT,
    FONT_SYMBOL_RIGHT_SHIFT,
} FontSymbolShift;

const uint8_t * fontGetNumberArray(uint8_t number);
const uint8_t * fontGetCharArray(char c);
const uint8_t * fontGetSpaceArray(void);
uint8_t * fontGetNumberArrayShifted(uint8_t number, FontSymbolShift shiftDirection, uint8_t shift);
uint8_t * fontAddDots(const uint8_t fontSymbol[FONT_SYMBOL_SIZE_IN_BYTES], FontSymbolShift dotsSide);