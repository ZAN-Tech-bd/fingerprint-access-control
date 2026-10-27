#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include "common.h"

SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  showFlipped("STARTING", "");
  delay(1000);

  finger.begin(57600);
  if (finger.verifyPassword()) {
    showFlipped("SENSOR OK", "");
  } else {
    showFlipped("SENSOR ERROR", "CHECK WIRING");
    while (1) { delay(1); }
  }
  delay(1500);
  resetScreen();
}

void loop() {
  int result = scanFingerprint();
  if (result == -2) return;            // no finger on the sensor

  if (result == -1) {
    showFlipped("ACCESS", "DENIED");
    Serial.println(F("Access DENIED"));
  } else {
    showGranted(result);
  }

  delay(2500);
  waitForFingerRemoved();
  resetScreen();
}

void showGranted(int id) {
  char name[NAME_MAX + 1];
  Serial.print(F("Access GRANTED: ID "));
  Serial.print(id);

  if (loadStaff(id, name)) {
    Serial.print(F(" - "));
    Serial.println(name);
    // Use the longest second line that still fits the 8-character limit
    if      (fitsFlipped(name, "GRANTED")) showFlipped(name, "GRANTED");
    else if (fitsFlipped(name, "OK"))      showFlipped(name, "OK");
    else                                   showFlipped(name, "");
  } else {
    Serial.println(F(" (no name saved)"));
    char buf[17];
    if (id < 10) {
      snprintf(buf, sizeof(buf), "GRANTED ID %d", id);
      showFlipped("ACCESS", buf);
    } else {
      snprintf(buf, sizeof(buf), "ID %d", id);
      showFlipped("OK", buf);
    }
  }
}

void resetScreen() {
  showFlipped("READY", "SCAN FINGER");
}

void waitForFingerRemoved() {
  while (finger.getImage() != FINGERPRINT_NOFINGER) delay(50);
}

int scanFingerprint() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) return -2;
  if (p != FINGERPRINT_OK) return -1;
  if (finger.image2Tz() != FINGERPRINT_OK) return -1;
  if (finger.fingerFastSearch() != FINGERPRINT_OK) return -1;
  return finger.fingerID;
}
