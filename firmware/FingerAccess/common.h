#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // try 0x3F if blank

// ===================== STAFF STORAGE (EEPROM) =====================
#define MAX_STAFF  50      // fingerprint IDs 1..50
#define NAME_MAX   14      // max characters in a name
#define REC_SIZE   16      // 1 marker byte + 15 name bytes
#define REC_VALID  0xA5

int recAddr(uint8_t id) { return (id - 1) * REC_SIZE; }

bool loadStaff(uint8_t id, char *name) {        // name buffer: NAME_MAX + 1
  name[0] = '\0';
  if (id < 1 || id > MAX_STAFF) return false;
  int a = recAddr(id);
  if (EEPROM.read(a) != REC_VALID) return false;
  for (uint8_t i = 0; i < NAME_MAX; i++) name[i] = EEPROM.read(a + 1 + i);
  name[NAME_MAX] = '\0';
  return true;
}

void saveStaff(uint8_t id, const char *name) {
  int a = recAddr(id);
  bool ended = false;
  for (uint8_t i = 0; i < NAME_MAX + 1; i++) {
    char ch = ended ? 0 : name[i];
    if (ch == 0) ended = true;
    EEPROM.update(a + 1 + i, ch);
  }
  EEPROM.update(a, REC_VALID);
}

void deleteStaff(uint8_t id) {
  if (id >= 1 && id <= MAX_STAFF) EEPROM.update(recAddr(id), 0xFF);
}

// ===================== FLIPPED LCD (180 degrees) =====================
const uint8_t FONT[36][7] PROGMEM = {
  {0b01110,0b10001,0b10001,0b10001,0b11111,0b10001,0b10001}, // A
  {0b11110,0b10001,0b10001,0b11110,0b10001,0b10001,0b11110}, // B
  {0b01110,0b10001,0b10000,0b10000,0b10000,0b10001,0b01110}, // C
  {0b11100,0b10010,0b10001,0b10001,0b10001,0b10010,0b11100}, // D
  {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b11111}, // E
  {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b10000}, // F
  {0b01110,0b10001,0b10000,0b10111,0b10001,0b10001,0b01111}, // G
  {0b10001,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001}, // H
  {0b01110,0b00100,0b00100,0b00100,0b00100,0b00100,0b01110}, // I
  {0b00111,0b00010,0b00010,0b00010,0b00010,0b10010,0b01100}, // J
  {0b10001,0b10010,0b10100,0b11000,0b10100,0b10010,0b10001}, // K
  {0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111}, // L
  {0b10001,0b11011,0b10101,0b10101,0b10001,0b10001,0b10001}, // M
  {0b10001,0b10001,0b11001,0b10101,0b10011,0b10001,0b10001}, // N
  {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110}, // O
  {0b11110,0b10001,0b10001,0b11110,0b10000,0b10000,0b10000}, // P
  {0b01110,0b10001,0b10001,0b10001,0b10101,0b10010,0b01101}, // Q
  {0b11110,0b10001,0b10001,0b11110,0b10100,0b10010,0b10001}, // R
  {0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110}, // S
  {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100}, // T
  {0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110}, // U
  {0b10001,0b10001,0b10001,0b10001,0b10001,0b01010,0b00100}, // V
  {0b10001,0b10001,0b10001,0b10101,0b10101,0b10101,0b01010}, // W
  {0b10001,0b10001,0b01010,0b00100,0b01010,0b10001,0b10001}, // X
  {0b10001,0b10001,0b10001,0b01010,0b00100,0b00100,0b00100}, // Y
  {0b11111,0b00001,0b00010,0b00100,0b01000,0b10000,0b11111}, // Z
  {0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110}, // 0
  {0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110}, // 1
  {0b01110,0b10001,0b00001,0b00010,0b00100,0b01000,0b11111}, // 2
  {0b11111,0b00010,0b00100,0b00010,0b00001,0b10001,0b01110}, // 3
  {0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010}, // 4
  {0b11111,0b10000,0b11110,0b00001,0b00001,0b10001,0b01110}, // 5
  {0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b01110}, // 6
  {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}, // 7
  {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}, // 8
  {0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b01100}  // 9
};

const char SYMMETRIC[] = " SNIOHXZ08";   // look the same rotated 180 degrees

bool isSymmetric(char ch) {
  for (uint8_t i = 0; SYMMETRIC[i]; i++) if (SYMMETRIC[i] == ch) return true;
  return false;
}

bool isFlippable(char ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ' ';
}

uint8_t rev5(uint8_t b) {
  uint8_t r = 0;
  for (uint8_t i = 0; i < 5; i++) { r = (r << 1) | (b & 1); b >>= 1; }
  return r;
}

bool buildFlippedGlyph(char ch, uint8_t *out) {
  int idx;
  if (ch >= 'A' && ch <= 'Z') idx = ch - 'A';
  else if (ch >= '0' && ch <= '9') idx = 26 + (ch - '0');
  else return false;
  for (uint8_t r = 0; r < 7; r++) out[r] = rev5(pgm_read_byte(&FONT[idx][6 - r]));
  out[7] = 0;
  return true;
}

void normalizeLine(const char *src, char *dst) {
  uint8_t i = 0;
  for (; i < 16 && src[i]; i++) dst[i] = toupper(src[i]);
  for (; i < 16; i++) dst[i] = ' ';
  dst[16] = '\0';
}

// Number of custom character slots a screen needs (more than 8 = won't fit)
uint8_t slotsNeeded(const char *top, const char *bottom) {
  char lines[2][17];
  normalizeLine(top, lines[0]);
  normalizeLine(bottom, lines[1]);
  char seen[32];
  uint8_t n = 0;
  for (uint8_t l = 0; l < 2; l++) {
    for (uint8_t c = 0; c < 16; c++) {
      char ch = lines[l][c];
      if (isSymmetric(ch)) continue;
      if (!isFlippable(ch)) return 99;
      bool found = false;
      for (uint8_t s = 0; s < n; s++) if (seen[s] == ch) found = true;
      if (!found) seen[n++] = ch;
    }
  }
  return n;
}

bool fitsFlipped(const char *top, const char *bottom) {
  return slotsNeeded(top, bottom) <= 8;
}

// top = the line the viewer sees on top
void showFlipped(const char *top, const char *bottom) {
  char lines[2][17];
  normalizeLine(top, lines[0]);
  normalizeLine(bottom, lines[1]);

  char slotChar[8];
  uint8_t used = 0, glyph[8];

  lcd.clear();

  for (uint8_t l = 0; l < 2; l++) {
    for (uint8_t c = 0; c < 16; c++) {
      char ch = lines[l][c];
      if (isSymmetric(ch)) continue;
      bool found = false;
      for (uint8_t s = 0; s < used; s++) if (slotChar[s] == ch) found = true;
      if (found) continue;
      if (used < 8 && buildFlippedGlyph(ch, glyph)) {
        lcd.createChar(used, glyph);
        slotChar[used++] = ch;
      }
    }
  }

  for (uint8_t l = 0; l < 2; l++) {
    for (uint8_t c = 0; c < 16; c++) {
      char ch = lines[l][c];
      lcd.setCursor(15 - c, 1 - l);
      if (isSymmetric(ch)) { lcd.write(ch); continue; }
      int8_t slot = -1;
      for (uint8_t s = 0; s < used; s++) if (slotChar[s] == ch) slot = s;
      lcd.write(slot >= 0 ? (uint8_t)slot : ' ');
    }
  }
}
