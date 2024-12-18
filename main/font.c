#include "font.h"

#define SPACE 0
/* https://github.com/squix78/MAX7219LedMatrix */
/* can be sent without transformations */
static const uint8_t fontNumberArray[][8] = {
    {0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x00}, // 0x30 // 0
    {0x30, 0xF0, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00}, // 0x31 // 1
    {0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00}, // 0x32 // 2
    {0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00}, // 0x33 // 3
    {0x1C, 0x3C, 0x6C, 0xCC, 0xFC, 0x0C, 0x0C, 0x00}, // 0x34 // 4
    {0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00}, // 0x35 // 5
    {0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00}, // 0x36 // 6
    {0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x00}, // 0x37 // 7
    {0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00}, // 0x38 // 8
    {0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00}, // 0x39 // 9
};

/* https://github.com/watterott/RPi-WS2812-HAT/blob/master/software/Text.ino */
static const uint8_t fontCharArray[][8] = {
    {0x30,0x78,0xCC,0xCC,0xFC,0xCC,0xCC,0x00}, // 0x41
    {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00}, // 0x42
    {0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00}, // 0x43
    {0xFC,0x6C,0x66,0x66,0x66,0x6C,0xFC,0x00}, // 0x44
    {0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00}, // 0x45
    {0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00}, // 0x46
    {0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00}, // 0x47
    {0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,0xCC,0x00}, // 0x48
    {0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00}, // 0x49
    {0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00}, // 0x4A
    {0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00}, // 0x4B
    {0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00}, // 0x4C
    {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00}, // 0x4D
    {0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00}, // 0x4E
    {0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00}, // 0x4F
    {0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00}, // 0x50
    {0x78,0xCC,0xCC,0xCC,0xDC,0x78,0x1C,0x00}, // 0x51
    {0xFC,0x66,0x66,0x7C,0x78,0x6C,0xE6,0x00}, // 0x52
    {0x78,0xCC,0xE0,0x38,0x1C,0xCC,0x78,0x00}, // 0x53
    {0xFC,0xB4,0x30,0x30,0x30,0x30,0x78,0x00}, // 0x54
    {0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFC,0x00}, // 0x55
    {0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x00}, // 0x56
    {0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00}, // 0x57
    {0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00}, // 0x58
    {0xCC,0xCC,0xCC,0x78,0x30,0x30,0x78,0x00}, // 0x59
    {0xFE,0xCC,0x98,0x30,0x62,0xC6,0xFE,0x00}, // 0x5A
    {0x78,0x60,0x60,0x60,0x60,0x60,0x78,0x00}, // 0x5B
};
        
static const uint8_t fontSpaceArray[8] = 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Space

static uint8_t symbol[FONT_SYMBOL_SIZE_IN_BYTES] = {0};

const uint8_t * fontGetNumberArray(uint8_t number)
{
    number &= 0x0F;
    return &fontNumberArray[number][0];
}

const uint8_t * fontGetCharArray(char c)
{
    return &fontCharArray[c - 'A'][0];
}

uint8_t * fontGetNumberArrayShifted(uint8_t number, FontSymbolShift shiftDirection, uint8_t shift)
{
    number &= 0x0F;
    for (uint8_t i = 0; i < FONT_SYMBOL_SIZE_IN_BYTES; i++) {
        switch (shiftDirection) {
        case FONT_SYMBOL_LEFT_SHIFT:
            symbol[i] = fontNumberArray[number][i] << shift;
            break;
        case FONT_SYMBOL_RIGHT_SHIFT:
            symbol[i] = fontNumberArray[number][i] >> shift;
            break;
        default:
            break;
        }
    }
    return symbol;
}

uint8_t * fontAddDots(const uint8_t fontSymbol[FONT_SYMBOL_SIZE_IN_BYTES], FontSymbolShift dotsSide)
{
    uint8_t orValue = 0x00;
    switch (dotsSide) {
    case FONT_SYMBOL_LEFT_SHIFT:
        orValue = 0x80;
        break;
    case FONT_SYMBOL_RIGHT_SHIFT:
        orValue = 0x01;
        break;
    default:
        break;
    }
    
    for (uint8_t i = 0; i < FONT_SYMBOL_SIZE_IN_BYTES; i++) {
        if (i == 1 || i == 2 || i == 5 || i == 6)
            symbol[i] = (fontSymbol[i] | orValue);
        else
            symbol[i] = fontSymbol[i];
    }
    return symbol;
}

const uint8_t * fontGetSpaceArray(void)
{
    return fontSpaceArray; 
}
