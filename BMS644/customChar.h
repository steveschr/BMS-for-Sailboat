/*
uint8_t  heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
*/
uint8_t  bell[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000
};

uint8_t  smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};
uint8_t  frownie[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b10001
};
uint8_t  anchor[] = {
  0b00100,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};

uint8_t  upArrow[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

uint8_t  downArrow[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

uint8_t  emptyBell[] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B11011,
  B00000,
  B00100,
  B00000
};

byte emptyBox[] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};
/*
byte degree[] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000
};
*/
byte locked[] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B11111,
  B11011,
  B11011,
  B11111
};

uint8_t  smileFace = 0;  //
uint8_t  lockedFace = 1;  //
uint8_t  anchorFace = 2; //
uint8_t  bellFace = 3;  //
uint8_t  downArrowFace = 4;
uint8_t  emptyBoxFace = 5;
uint8_t  upArrowFace = 6;
uint8_t  emptyBellFace = 7;
