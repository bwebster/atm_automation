#include "Matrix.h"

Matrix::Matrix() {
  setup = false;  
}

void Matrix::number(int v) {
  if (!setup) {
    matrix.begin();
    setup = true;
  }

  clearFrame();
  addToFrame('0' + v, 4);
  displayFrame();
}

void Matrix::letter(char c) {
  if (!setup) {
    matrix.begin();
    setup = true;
  }

  clearFrame();
  addToFrame(c, 4);
  displayFrame();
}

void Matrix::letterDelay(char c, int sec) {
  letter(c);
  delay(sec);
}

void Matrix::ok() {
  letterDelay('O', 400);
  letterDelay('K', 2000);
}

void Matrix::clearFrame() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 12; col++) {
      frame[row][col] = 0;
    }
  }
}

void Matrix::displayFrame() {
  matrix.renderBitmap(frame, 8, 12);
}

void Matrix::addToFrame(char c, int pos) {
  if (!setup) {
    matrix.begin();
    setup = true;
  }

  int index = -1;
  if (c >= '0' && c <= '9')
    index = c - '0';
  else if (c >= 'A' && c <= 'Z')
    index = c - 'A' + 10;
  else {
    Serial.print("WARNING: unsupported character: ");
    Serial.println(c);
    return;
  }

  for (int row = 0; row < 8; row++) {
    uint32_t temp = fonts[index][row] << (7 - pos);
    for (int col = 0; col < 12; col++) {
      frame[row][col] |= (temp >> (11 - col)) & 1;
    }
  }
}