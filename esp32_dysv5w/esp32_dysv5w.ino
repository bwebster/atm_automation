#include <HardwareSerial.h>
#include <Arduino.h>
#include <DYPlayerArduino.h>

constexpr uint32_t BAUD = 9600;
constexpr uint8_t RX_PIN = 25;   // HIGH on this pin starts track 1
constexpr uint8_t TX_PIN = 33;

HardwareSerial MP3Serial(2);
DY::Player player(&MP3Serial);
int track = 1;
int numTracks = 3;

void setup() {
  Serial.begin(115200);
  delay(2000);
  while (!Serial)
    ;

  Serial.println("Setting up pins");
  pinMode(RX_PIN, INPUT_PULLDOWN);   // RX from controller; HIGH to start sound
  pinMode(TX_PIN, OUTPUT);           // TX to controller; HIGH by default, LOW when playing (simulates busy pin)
  digitalWrite(TX_PIN, HIGH);

  Serial.println("Initializing mp3 module");
  MP3Serial.begin(BAUD, SERIAL_8N1, 21, 22);   // RX, TX
  delay(800);                  // allow module to boot
  player.begin();
  player.setVolume(25);        // 0…30
}

void loop() {
  static bool playing = false;
  static bool lastTrig = LOW;

  bool trig = digitalRead(RX_PIN);
  if (lastTrig == LOW && trig == HIGH) { /* rising edge: LOW->HIGH */
    Serial.print("Start track");
    Serial.println(track);
    digitalWrite(TX_PIN, LOW);
    player.playSpecified(track);
    playing = true;
    track += 1;
    if (track > numTracks) {
      track = 1;
    }
  }
  lastTrig = trig;

  if (playing) { 
    DY::PlayState st = player.checkPlayState();
    if (st == DY::PlayState::Stopped) {
      Serial.println("Stopped");
      digitalWrite(TX_PIN, HIGH);
      playing = false;
    }
  }
}
