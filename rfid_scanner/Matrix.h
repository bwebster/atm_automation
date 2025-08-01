#ifndef MATRIX_H
#define MATRIX_H

#include <Arduino.h>
// External: https://github.com/arduino-libraries/ArduinoGraphics
#include <ArduinoGraphics.h>
#include "Arduino_LED_Matrix.h"
#include "fonts.h"

class Matrix {
private:
  ArduinoLEDMatrix matrix;
  bool setup;
  uint8_t frame[8][12] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  };

  void clearFrame();
  void displayFrame();
  void addToFrame(char c, int pos);


public:
  Matrix();
  void start();
  void number(int v);
  void letter(char c);
  void letterDelay(char c, int sec);
  void ok();
};

#endif