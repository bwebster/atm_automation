#include <HardwareSerial.h>
#include <Arduino.h>
#include <DYPlayerArduino.h>

constexpr uint32_t BAUD = 9600;
constexpr uint8_t TRIGGER_PIN = 25;   // HIGH on this pin starts track 1
constexpr uint8_t DONE_PIN = 33;

HardwareSerial MP3Serial(2);
DY::Player player(&MP3Serial);

void setup() {
  Serial.begin(115200);
  delay(2000);
  while (!Serial)
    ;

  pinMode(TRIGGER_PIN, INPUT); 
  pinMode(DONE_PIN, OUTPUT);
  digitalWrite(DONE_PIN, LOW);

  MP3Serial.begin(BAUD, SERIAL_8N1, 21, 22);   // RX, TX
  delay(800);                  // allow module to boot
  player.begin();
  player.setVolume(25);        // 0…30
  // player.playSpecified(1);
  // Serial.println("Playing track 1");
}

void loop() {
  static bool lastTrig = HIGH;
  bool trig = digitalRead(TRIGGER_PIN);
  /* Rising edge: LOW → HIGH */
  if (lastTrig == HIGH && trig == LOW) {
    digitalWrite(DONE_PIN, LOW);
    Serial.println("Start track 1");
    player.playSpecified(1);
  }
  lastTrig = trig;

  static DY::PlayState state = DY::PlayState::Stopped;
  DY::PlayState st = player.checkPlayState();
  if (st != state) {
    switch (st) {
      case DY::PlayState::Playing:
        Serial.println("Playing");
        break;

      case DY::PlayState::Stopped:
        Serial.println("Stopped");
        digitalWrite(DONE_PIN, HIGH);
        break;

      case DY::PlayState::Paused:
        Serial.println("Paused");
        break;

      case DY::PlayState::Fail:
        Serial.println("Fail");
        break;
    }
  }
  state = st;
}
