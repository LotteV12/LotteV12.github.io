#include <LiquidCrystal.h>
#include <Wire.h>
#include "RTClib.h"
#include <ArduinoWiFi.h>
#include <PubSubClient.h>


RTC_DS3231 rtc;

int ahours = 0;
int aminute = 0;
int aday = 0;
int amonth = 0;
int ayear = 2017;

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

int spkrPin = 10;//speaker
int offPin = 12;//multi-use button
int menuPin = 13;//button to bring up menu or select
int upPin = 8;//button to increase value
int downPin = 9;//pin to decrease value

#define TOPICSETDATE "settings/date"
#define TOPICSETTIME "settings/time"

//wifi
char ssid[] = "telenet-1C060";
char pass[] = "xxx";

char daysOfTheWeek[7][12] = {"Zo", "Ma", "Di", "Woe", "Do", "Vr", "Za"};
String menuText[] = {"Set alarm", "View alarms"};//list of items in menu

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
  Serial.begin(9600);

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

void loop()
{
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
      if (item != 1) {
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.print(menuText[item + 1]);
      }
    }
    last = item;
    if (readButton(menuPin)) {
      return item;
    }
    if (readButton(downPin) && item < 1
       ) {
      item++;
      delay(50);
    }
    if (readButton(upPin) && item > 0) {
      item--;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();
  return -1;
}

void playAlarm() {
  tone(spkrPin, 367, 500);
      delay(600);
      if (readButton(offPin))
        return; 
        //noTone(spkrPin);
      tone(spkrPin, 343, 250);
      delay(250);
      if (readButton(offPin))
        return;
      tone(spkrPin, 270, 250);
      delay(320);
      if (readButton(offPin))
        return;
      tone(spkrPin, 310, 2000);
      delay(2500);
      if (readButton(offPin))
        return;
}

void stopAlarm() {
  noTone(spkrPin);
}

void checkAlarms() {
  DateTime now = rtc.now();

  if (now.hour() == ahours && now.minute() == aminute && now.day() == aday && now.month() == amonth && now.year() == ayear) {
    playAlarm();
  }
}

void dispTime(boolean time) {

  DateTime now = rtc.now();

  int hours = now.hour();
  int minutes = now.minute();

  lcd.setCursor(5, 0);

  if (hours < 10) {
    lcd.print("0");
    lcd.print(hours);
  } else {
    lcd.print(hours);
  }

  lcd.print(":");
  if (minutes < 10) {
    lcd.print("0");
    lcd.print(minutes);
  } else {
    lcd.print(minutes);
  }

  checkAlarms();
  dispDay();

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

void setAlarm() {
  lcd.clear();
  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 0);
  lcd.print("Set hour");
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    lcd.print(ahours);
    if (readButton(upPin) && ahours < 23) {
      ahours++;
      delay(50);
    }
    if (readButton(downPin) && ahours > 0) {
      ahours --;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Set minute");
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    lcd.print(aminute);
    if (readButton(upPin) && aminute < 59) {
      aminute++;
      delay(50);
    }
    if (readButton(downPin) && aminute > 0) {
      aminute--;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();

  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 0);
  lcd.print("Set day");
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    lcd.print(aday);
    if (readButton(upPin) && aday < 31) {
      aday++;
      delay(50);
    }
    if (readButton(downPin) && aday > 0) {
      aday --;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();

  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 0);
  lcd.print("Set month");
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    lcd.print(amonth);
    if (readButton(upPin) && amonth < 12) {
      amonth++;
      delay(50);
    }
    if (readButton(downPin) && amonth > 0) {
      amonth --;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();

  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 0);
  lcd.print("Set year");
  while (!readButton(menuPin)) {
    lcd.setCursor(0, 1);
    lcd.print(ayear);
    if (readButton(upPin) && ayear < 2020) {
      ayear++;
      delay(50);
    }
    if (readButton(downPin) && ayear > 0) {
      ayear --;
      delay(50);
    }
    delay(50);
  }
  lcd.clear();

}

void viewAlarm() {
  lcd.clear();
  while (readButton(menuPin)) {
    delay(20);
  }
  lcd.setCursor(0, 1);

  while (!readButton(offPin)) {
    lcd.setCursor(0, 0);
    lcd.print(ahours);
    lcd.print(":");
    lcd.print(aminute);

    lcd.print(" ");

    lcd.print(aday);
    lcd.print("/");
    lcd.print(amonth);
    lcd.print("/");
    lcd.print(ayear);
  }
  lcd.clear();
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
