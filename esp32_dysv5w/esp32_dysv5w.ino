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

  Serial.println("Setting up pins");
  pinMode(TRIGGER_PIN, INPUT_PULLDOWN); 
  pinMode(DONE_PIN, OUTPUT);
  digitalWrite(DONE_PIN, LOW);

  Serial.println("Initializing mp3 module");
  MP3Serial.begin(BAUD, SERIAL_8N1, 21, 22);   // RX, TX
  delay(800);                  // allow module to boot
  player.begin();
  player.setVolume(25);        // 0…30
}

void loop() {
  static bool playing = false;
  static bool lastTrig = LOW;

  bool trig = digitalRead(TRIGGER_PIN);
  if (lastTrig == LOW && trig == HIGH) { /* rising edge: LOW->HIGH */
    Serial.println("Start track 1");
    digitalWrite(DONE_PIN, LOW);
    player.playSpecified(1);
    playing = true;
  }
  lastTrig = trig;

  if (playing) { 
    DY::PlayState st = player.checkPlayState();
    if (st == DY::PlayState::Stopped) {
      Serial.println("Stopped");
      digitalWrite(DONE_PIN, HIGH);
    }
  }
}
