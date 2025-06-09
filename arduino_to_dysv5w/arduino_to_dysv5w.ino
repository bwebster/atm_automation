#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DYPlayerArduino.h>

#define TX_PIN 5
#define RX_PIN 4
#define BUSY_PIN 3
#define BAUD 9600

SoftwareSerial mp3Serial(RX_PIN, TX_PIN);
DY::Player     player(&mp3Serial);

unsigned long  playStartMs = 0;       // millis() when we started the track
bool           waitingToStop = false; // true while 5-s watchdog is armed

void setup() {
  Serial.begin(115200);
  delay(2000);
  while (!Serial)
    ;

  Serial.println("Init DY-HV8F ...");

  mp3Serial.begin(BAUD);
  player.begin();
  delay(500);
  player.setVolume(25);

  pinMode(BUSY_PIN, INPUT_PULLUP);

  player.playSpecified(1);
  playStartMs     = millis();
  waitingToStop   = true;
  Serial.println("Started 00001.mp3 – watchdog armed for +5 s");
}

void loop() {
  static bool last = HIGH;
  bool now = digitalRead(BUSY);

  if (last == HIGH && now == LOW)  Serial.println(F("► started"));
  if (last == LOW  && now == HIGH) Serial.println(F("■ finished"));

  last = now;

  // DY::PlayState st = player.checkPlayState();

  // if (st == DY::PlayState::Playing) {
  //   Serial.println("Playing");
  // } else if (st == DY::PlayState::Paused) {
  //   Serial.println("Paused");
  // } else if (st == DY::PlayState::Stopped) {
  //   Serial.println("Stopped");
  // } else {
  //   Serial.println("Failed");
  // }

// // ----- 1. fire the 5-second watchdog ---------------------------------
//   if (waitingToStop &&
//       (millis() - playStartMs >= 10000))           // 5 000 ms elapsed
//   {
//     if (st == DY::PlayState::Playing) {
//       Serial.println("⏹️  5 s elapsed – sending stop()");
//       player.stop();
//     }
//     waitingToStop = false;                        // watchdog disarmed
//   }

//   // ----- 2. decide what to do after stop --------------------------------
//   if (st == DY::PlayState::Stopped && !waitingToStop) {
//     Serial.println("Restarting track and re-arming watchdog");
//     player.playSpecified(1);
//     playStartMs   = millis();
//     waitingToStop = true;
//   }

//   // ----- 3. comm failure recovery --------------------------------------
//   if (st == DY::PlayState::Fail) {
//     Serial.println("Comm FAIL – reinit");
//     player.begin();
//     delay(300);
//     player.setVolume(25);
//     player.playSpecified(1);
//     playStartMs   = millis();
//     waitingToStop = true;
//   }

//   delay(100);     // small loop delay keeps serial traffic light
}
