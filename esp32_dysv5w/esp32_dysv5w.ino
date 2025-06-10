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

<<<<<<< Updated upstream
  pinMode(TRIGGER_PIN, INPUT); 
  pinMode(DONE_PIN, OUTPUT);
  digitalWrite(DONE_PIN, LOW);

=======
  Serial.println("Setting up pins");
  pinMode(TRIGGER_PIN, INPUT_PULLDOWN); 
  pinMode(DONE_PIN, OUTPUT);
  digitalWrite(DONE_PIN, LOW);

  Serial.println("Initializing mp3 module");
>>>>>>> Stashed changes
  MP3Serial.begin(BAUD, SERIAL_8N1, 21, 22);   // RX, TX
  delay(800);                  // allow module to boot
  player.begin();
  player.setVolume(25);        // 0…30
<<<<<<< Updated upstream
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
=======
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
>>>>>>> Stashed changes
}
