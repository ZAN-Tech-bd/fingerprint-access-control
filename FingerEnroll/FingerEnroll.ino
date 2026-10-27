#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include "common.h"

SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// ---------- Serial input helpers ----------
void readLine(char *buf, uint8_t size) {
  uint8_t i = 0;
  unsigned long lastChar = 0;
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      lastChar = millis();
      if (c == '\r') continue;
      if (c == '\n') break;
      if (i < size - 1) buf[i++] = c;
    } else if (i > 0 && millis() - lastChar > 200) {
      break;   // also works with "No line ending"
    }
  }
  buf[i] = '\0';
  Serial.print(F("> "));
  Serial.println(buf);
}

int readNumber() {
  char buf[12];
  readLine(buf, sizeof(buf));
  return atoi(buf);
}

void printMenu() {
  Serial.println();
  Serial.println(F("====== STAFF REGISTRATION ======"));
  Serial.println(F(" e = Enroll new staff"));
  Serial.println(F(" l = List staff"));
  Serial.println(F(" d = Delete staff"));
  Serial.println(F(" x = Delete ALL staff"));
  Serial.println(F("================================"));
}

// ---------- Fingerprint helpers ----------
bool waitForFinger(uint16_t timeoutMs) {
  unsigned long t = millis();
  while (millis() - t < timeoutMs) {
    if (finger.getImage() == FINGERPRINT_OK) return true;
    delay(50);
  }
  return false;
}

void waitForFingerRemoved() {
  while (finger.getImage() != FINGERPRINT_NOFINGER) delay(50);
}

bool enrollFinger(uint8_t id) {
  // First scan
  showFlipped("PLACE FINGER", "");
  Serial.println(F("Place finger on sensor..."));
  if (!waitForFinger(15000)) { Serial.println(F("Timeout.")); return false; }
  if (finger.image2Tz(1) != FINGERPRINT_OK) { Serial.println(F("Bad image. Try again.")); return false; }

  // Block the same finger being saved under two IDs
  if (finger.fingerFastSearch() == FINGERPRINT_OK && finger.fingerID != id) {
    char other[NAME_MAX + 1];
    Serial.print(F("This finger is already saved as ID "));
    Serial.print(finger.fingerID);
    if (loadStaff(finger.fingerID, other)) { Serial.print(F(" (")); Serial.print(other); Serial.print(F(")")); }
    Serial.println();
    return false;
  }

  showFlipped("REMOVE FINGER", "");
  Serial.println(F("Remove finger."));
  delay(1000);
  waitForFingerRemoved();

  // Second scan
  showFlipped("PLACE AGAIN", "");
  Serial.println(F("Place the SAME finger again..."));
  if (!waitForFinger(15000)) { Serial.println(F("Timeout.")); return false; }
  if (finger.image2Tz(2) != FINGERPRINT_OK) { Serial.println(F("Bad image. Try again.")); return false; }

  if (finger.createModel() != FINGERPRINT_OK) { Serial.println(F("Scans did not match.")); return false; }
  if (finger.storeModel(id) != FINGERPRINT_OK) { Serial.println(F("Could not store on sensor.")); return false; }
  return true;
}

// ---------- Name input + validation ----------
bool askName(char *name) {
  Serial.print(F("Enter staff name (max "));
  Serial.print(NAME_MAX);
  Serial.println(F(" chars, A-Z 0-9 space):"));

  char buf[32];
  readLine(buf, sizeof(buf));

  // trim and uppercase
  char *s = buf;
  while (*s == ' ') s++;
  uint8_t len = strlen(s);
  while (len > 0 && s[len - 1] == ' ') s[--len] = '\0';

  if (len == 0 || len > NAME_MAX) { Serial.println(F("Name length invalid.")); return false; }
  for (uint8_t i = 0; i < len; i++) {
    s[i] = toupper(s[i]);
    if (!isFlippable(s[i])) { Serial.println(F("Only A-Z, 0-9 and space allowed.")); return false; }
  }

  if (!fitsFlipped(s, "")) {
    Serial.println(F("Too many different letters for the flipped LCD."));
    Serial.println(F("Max 8 different letters (S N I O H X Z 0 8 don't count). Use a short name/nickname."));
    return false;
  }
  if (!fitsFlipped(s, "OK")) {
    Serial.println(F("Note: the access screen will show the name only."));
  }

  strcpy(name, s);
  return true;
}

// ---------- Menu actions ----------
void enrollFlow() {
  Serial.print(F("Enter ID (1-"));
  Serial.print(MAX_STAFF);
  Serial.println(F("):"));
  int id = readNumber();
  if (id < 1 || id > MAX_STAFF) { Serial.println(F("Invalid ID.")); return; }

  char existing[NAME_MAX + 1];
  if (loadStaff(id, existing)) {
    Serial.print(F("ID already used by "));
    Serial.print(existing);
    Serial.println(F(". Overwrite? (y/n)"));
    char buf[4];
    readLine(buf, sizeof(buf));
    if (tolower(buf[0]) != 'y') { Serial.println(F("Cancelled.")); return; }
  }

  char name[NAME_MAX + 1];
  if (!askName(name)) return;

  Serial.print(F("Enrolling ")); Serial.print(name);
  Serial.print(F(" as ID ")); Serial.println(id);

  if (enrollFinger(id)) {
    saveStaff(id, name);
    char line2[17];
    snprintf(line2, sizeof(line2), "ID %d", id);
    showFlipped("SAVED", line2);
    Serial.println(F("SUCCESS: staff saved."));
  } else {
    showFlipped("FAILED", "");
    Serial.println(F("FAILED: nothing saved."));
  }
  delay(2000);
  waitForFingerRemoved();
  showFlipped("ENROLL MODE", "");
}

void listStaff() {
  char name[NAME_MAX + 1];
  uint8_t count = 0;
  Serial.println(F("ID  | NAME"));
  Serial.println(F("----+---------------"));
  for (uint8_t id = 1; id <= MAX_STAFF; id++) {
    if (loadStaff(id, name)) {
      if (id < 10) Serial.print(' ');
      Serial.print(id); Serial.print(F("  | ")); Serial.println(name);
      count++;
    }
  }
  Serial.print(F("Total: ")); Serial.println(count);
}

void deleteFlow() {
  Serial.println(F("Enter ID to delete:"));
  int id = readNumber();
  if (id < 1 || id > MAX_STAFF) { Serial.println(F("Invalid ID.")); return; }
  char name[NAME_MAX + 1];
  bool hasName = loadStaff(id, name);
  Serial.print(F("Delete ID ")); Serial.print(id);
  if (hasName) { Serial.print(F(" (")); Serial.print(name); Serial.print(F(")")); }
  Serial.println(F("? (y/n)"));
  char buf[4];
  readLine(buf, sizeof(buf));
  if (tolower(buf[0]) != 'y') { Serial.println(F("Cancelled.")); return; }

  finger.deleteModel(id);
  deleteStaff(id);
  Serial.println(F("Deleted."));
}

void clearAllFlow() {
  Serial.println(F("Type YES to delete ALL fingerprints and names:"));
  char buf[8];
  readLine(buf, sizeof(buf));
  if (strcmp(buf, "YES") != 0) { Serial.println(F("Cancelled.")); return; }
  finger.emptyDatabase();
  for (uint8_t id = 1; id <= MAX_STAFF; id++) deleteStaff(id);
  Serial.println(F("All staff deleted."));
}

// ---------- Main ----------
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  finger.begin(57600);
  if (!finger.verifyPassword()) {
    showFlipped("SENSOR ERROR", "CHECK WIRING");
    Serial.println(F("Fingerprint sensor not found!"));
    while (1) { delay(1); }
  }
  showFlipped("ENROLL MODE", "");
  printMenu();
}

void loop() {
  char cmd[8];
  readLine(cmd, sizeof(cmd));
  switch (tolower(cmd[0])) {
    case 'e': enrollFlow();   break;
    case 'l': listStaff();    break;
    case 'd': deleteFlow();   break;
    case 'x': clearAllFlow(); break;
    default:  Serial.println(F("Unknown command."));
  }
  printMenu();
}
