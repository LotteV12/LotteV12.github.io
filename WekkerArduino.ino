#include <LiquidCrystal.h>
#include "RTClib.h"
#include <Wire.h>
#include <EEPROM.h>//included in default arduino compiler

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Zo", "Ma", "Di", "Woe", "Do", "Vr", "Za"};

String menuText[] = {"Set alarm", "Alarmen bekijken", "Alle alarmen af", "Actieve alarmen", "Reset alarmen", "12/24 uur"};//list of items in menu
String patternName[] = {":John Sena     ", ":Alarm sound B ", ":Alarm sound C ", ":Alarm sound D ", ":Oscillating A  ", ":Oscillating B  ", ":High pitch    ", ":Random tones  "};//list of alarm pattern names

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

int spkrPin = 11;//speaker
int offPin = 12;//multi-use button
int menuPin = 13;//button to bring up menu or select
int upPin = 8;//button to increase value
int downPin = 9;//pin to decrease value

int a = 0;//used in functions
int u1 = 0;//used in functions
int d1 = 0;//used in functions

int ahours = 0;
int aminute = 0;

byte arrow[8] = {//right pointing arrow used in menu
  0b00000,
  0b00100,
  0b00010,
  0b11111,
  0b00010,
  0b00100,
  0b00000,
  0b00000
};

void setup()
{
  rtc.begin();

  pinMode(offPin, INPUT_PULLUP);//initialize certain pins
  pinMode(spkrPin, OUTPUT);
  pinMode(menuPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.begin(16, 2);
  lcd.clear();
  lcd.createChar(0, arrow);
  randomSeed(millis());
}


void loop() {
  dispTime(0);
  delay(50);
  if (readButton(menuPin)) { //start menu if menupin pressed
    switch (menu()) {
      case 0: //set alarm
        setAlarm();
        dispTime(1);
        break;
      case 1: //view alarm
        viewAlarm();
        dispTime(1);
        break;
      case 2:
        allOff();
        dispTime(1);
        break;
      case 3: //view list of activated alarms
        DispalarmsOn();
        dispTime(1);
        break;
      case 4: //reset all alarms
        for (int k = 0; k < 503; k++) {
          if (EEPROM.read(k) != 0) {
            EEPROM.write(k, 0);
          }
        }
        dispTime(1);
        break;
      case 5:
        //12/24hr time
        if (EEPROM.read(504) != 2)
          EEPROM.write(504, 2);//24
        else
          EEPROM.write(504, 1);//12
        dispTime(1);
        break;
      case -1: //cancel
        dispTime(1);
        break;
    }
  }
}

int menu() { // return -1 for close and other
  lcd.clear();
  int item = 0;
  int last = -1;

  while (readButton(menuPin)) {
    delay(20);
  }

  while (!readButton(offPin)) { //close menu with off pin
    if (item != last) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.write((byte)0);
      lcd.print(menuText[item]);
      if (item != 5) {
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.print(menuText[item + 1]);
      }
    }
    last = item;
    if (readButton(menuPin)) {
      return item;
    }
    if (readButton(downPin) && item < 5) {
      item++;
      delay(50);
    }
    if (readButton(upPin) && item > 0) {
      item--;
      delay(50);
    }
    delay(50);
  }
  return -1;
}

void turnOn(int num) { //turn selected alarm on
  EEPROM.write(num * 13 + 1, 1);
}

void dispTime(boolean time) {

  DateTime now = rtc.now();

  lcd.clear();
  lcd.setCursor(5, 0);
  int hr = EEPROM.read(504);
  if (hr == 2 && now.hour() < 10) {
    lcd.print("0");
  }
  if (hr == 1 && now.hour() > 12) {
    lcd.print(now.hour() - 12);
  }
  if (hr == 2)
    lcd.print(now.hour());

  if (hr == 1 && now.hour() == 0) {
    lcd.print("12");
  }
  if (hr == 1 && now.hour() < 12 && now.hour() != 0) {
    lcd.print(now.hour());
  }
  lcd.print(":");
  if (now.minute() < 10) {
    lcd.print("0");
    lcd.print(now.minute());
  } else {
    lcd.print(now.minute());
  }

  checkAlarms();
  dispDay();

}

boolean dayMatch(int a) { //check if alarm's day matches current day
  DateTime now = rtc.now();
  switch (now.dayOfTheWeek()) {
    case 1://Sunday
      if (EEPROM.read(a * 13 + 5) == 1)
        return 1;
      return 0;
    case 2://Monday
      if (EEPROM.read(a * 13 + 6) == 1)
        return 1;
      return 0;
    case 3://Tuesday
      if (EEPROM.read(a * 13 + 7) == 1)
        return 1;
      return 0;
    case 4://Wednesday
      if (EEPROM.read(a * 13 + 8) == 1)
        return 1;
      return 0;
    case 5://Thursday
      if (EEPROM.read(a * 13 + 9) == 1 )
        return 1;
      return 0;
    case 6://Friday
      if (EEPROM.read(a * 13 + 10) == 1 )
        return 1;
      return 0;
    case 7://Saturday
      if (EEPROM.read(a * 13 + 11) == 1)
        return 1;
      return 0;
  }
}

void playAlarm(int pat, int num) { //play alarm, cancelled by offPin
  int f = 0;
  if (num == 0)
    return ;

  tone(spkrPin, 635);
  delay(500);
  tone(spkrPin, 912);
  delay(500);

}

void checkAlarms() { //to be run once a minute

  DateTime now = rtc.now();

  for (int a = 0; a < 10; a++) { //go through alarmsOn
    if (EEPROM.read(a * 13 + 1) == 1) {
      if (dayMatch(a) && EEPROM.read(a * 13 + 12) > 0) {
        if (now.hour() == EEPROM.read(a * 13 + 2) && now.minute() == EEPROM.read(a * 13 + 3)) { //if time=alarm, play alarm
          lcd.setCursor(0, 1);
          lcd.print("Playing alarm:");
          lcd.print(a);
          EEPROM.write(a * 13 + 12, EEPROM.read(a * 13 + 12) - 1);
          playAlarm(EEPROM.read(a * 13 + 4), 50);
          break;
        }
      }
    }
  }
}

void dispDay() { //print date in d.m.y on second line

  DateTime now = rtc.now();

  lcd.setCursor(2, 1);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());
  lcd.print("    ");
}

void viewAlarm() { //complete sequence to view selected alarm
  lcd.clear();
  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 0);
  int last = -1;
  if (a == -1) {
    a = 0;
  }

  while (!readButton(offPin)) {
    lcd.setCursor(0, 0);
    if (last != a) {
      lcd.print("#");
      lcd.print(a);
      lcd.print(" ");
      if (EEPROM.read(a * 13 + 2) < 10)
        lcd.print("0");
      lcd.print(EEPROM.read(a * 13 + 2));
      lcd.print(":");
      if (EEPROM.read(a * 13 + 3) < 10)
        lcd.print("0");
      lcd.print(EEPROM.read(a * 13 + 3));
      lcd.print(" ");

      lcd.print(" Reps:");
      lcd.print(EEPROM.read(a * 13 + 12));
      if (EEPROM.read(a * 13 + 12) < 100)
        lcd.print("  ");

      lcd.setCursor(0, 1);
      if (EEPROM.read(a * 13 + 5) == 1)
        lcd.print("ZO");
      else
        lcd.print("zo");
      if (EEPROM.read(a * 13 + 6) == 1)
        lcd.print("MA");
      else
        lcd.print("ma");
      if (EEPROM.read(a * 13 + 7) == 1)
        lcd.print("DI");
      else
        lcd.print("di");
      if (EEPROM.read(a * 13 + 8) == 1)
        lcd.print("WO");
      else
        lcd.print("wo");
      if (EEPROM.read(a * 13 + 9) == 1)
        lcd.print("DO");
      else
        lcd.print("do");
      if (EEPROM.read(a * 13 + 10) == 1)
        lcd.print("VR");
      else
        lcd.print("vr");
      if (EEPROM.read(a * 13 + 11) == 1)
        lcd.print("ZA");
      else
        lcd.print("za");
    }
    last = a;
    if (readButton(menuPin)) {
      last = -1;
      if (EEPROM.read(a * 13 + 1) == 0)
        EEPROM.write(a * 13 + 1, 1);
      else
        EEPROM.write(a * 13 + 1, 0);
      delay(50);
    }
    if (readButton(upPin) && a < 9) {
      a++;
      delay(50);
    }
    if (readButton(downPin) && a > 0) {
      a--;
      delay(50);
    }
    delay(50);
  }

}

void setAlarm() { //complete sequence to set and activate selected alarm
  lcd.clear();
  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 0);
  lcd.print("Select alarm:");
  int last = -1;
  if (a == -1)
    a = 0;
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    if (last != a) {
      lcd.print(a);
      if (a < 10)
        lcd.print(" ");
    }
    last = a;
    if (readButton(upPin) && a < 4) {
      a++;
      delay(50);
    }
    if (readButton(downPin) && a > 0) {
      a--;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();
  while (readButton(menuPin))
    delay(20);
  lcd.setCursor(0, 0);
  lcd.print("Set hour:");
  int val = EEPROM.read(a * 13 + 2);
  last = -1;
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    if (last != val) {
      lcd.print(val);
      if (val < 10)
        lcd.print(" ");
    }
    last = val;
    if (readButton(upPin) && val < 23) {
      val++;
      delay(50);
    }
    if (readButton(downPin) && val > 0) {
      val--;
      delay(50);
    }
    delay(50);
  }
  EEPROM.write(a * 13 + 2, val);
  lcd.clear();
  while (readButton(menuPin)) {
    delay(20);
  }

  lcd.setCursor(0, 0);
  lcd.print("Set minute:");
  last = -1;
  val = EEPROM.read(a * 13 + 3);
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    if (last != val) {
      lcd.print(val);
      if (val < 10)
        lcd.print(" ");
    }
    last = val;
    if (readButton(upPin) && val < 59) {
      val++;
      delay(50);
    }
    if (readButton(downPin) && val > 0) {
      val--;
      delay(50);
    }
    delay(50);
  }
  EEPROM.write(a * 13 + 3, val);
  lcd.clear();
  while (readButton(menuPin))
    delay(20);
  lcd.setCursor(0, 0);
  lcd.print("Up: Zo on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Zo off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  if (u1)
    EEPROM.write(a * 13 + 5, 1);
  else
    EEPROM.write(a * 13 + 5, 0);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Up: Ma on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Ma off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  if (u1)
    EEPROM.write(a * 13 + 6, 1);
  else
    EEPROM.write(a * 13 + 6, 0);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Up: Di on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Di off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  if (u1)
    EEPROM.write(a * 13 + 7, 1);
  else
    EEPROM.write(a * 13 + 7, 0);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Up: Woe on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Woe off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  if (u1)
    EEPROM.write(a * 13 + 8, 1);
  else
    EEPROM.write(a * 13 + 8, 0);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Up: Do on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Do off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  if (u1)
    EEPROM.write(a * 13 + 9, 1);
  else
    EEPROM.write(a * 13 + 9, 0);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Up: Vr on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Vr off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  if (u1)
    EEPROM.write(a * 13 + 10, 1);
  else
    EEPROM.write(a * 13 + 10, 0);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Up: Za on");
  lcd.setCursor(0, 1);
  lcd.print("Down: Za off");
  do {
    u1 = readButton(upPin);
    d1 = readButton(downPin);
    delay(50);
  } while (!u1 && !d1);
  while (readButton(upPin) || readButton(downPin))
    delay(20);
  if (u1)
    EEPROM.write(a * 13 + 11, 1);
  else
    EEPROM.write(a * 13 + 11, 0);
  lcd.clear();



  lcd.clear();
  turnOn(a);
}

void allOff() { //turn all alarms off
  for (int k = 0; k < 4; k++) {
    if (EEPROM.read(k * 13 + 1) != 0)
      EEPROM.write(k * 13 + 1, 0);
  }
}

void DispalarmsOn() { //displays alarms turned on
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarms on:");
  for (int a = 0; a < 4; a++) {
    lcd.setCursor(11, 0);
    if (EEPROM.read(a * 13 + 1) == 1) {
      lcd.print(a);
      lcd.print(" ");
      delay(1000);
    }
  }
  while (readButton(offPin))
    delay(20);
}

bool readButton(int pin) { //debounce button and invert output
  if (digitalRead(pin)) {
    return 0;
  }
  else {
    delay(40);
    return 1;
  }
}
