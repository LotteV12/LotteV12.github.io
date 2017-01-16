#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Temboo.h>

//TEMBOO account info
#define TEMBOO_ACCOUNT "lotte"  // Your Temboo account name 
#define TEMBOO_APP_KEY_NAME "AlarmClock"  // Your Temboo app key name
#define TEMBOO_APP_KEY "WSRPZAOl9PabeAbROeWM8QUhRgCKWi9H"  // Your Temboo app key

#define DHTPIN 9   // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int spkrPin = 8;//speaker
int menuPin = A0;//button to bring up menu or select
int offPin = A1;//multi-use button
int downPin = A2;//pin to decrease value
int upPin = A3;//button to increase value

char daysOfTheWeek[7][12] = {"Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za"};

char ssid[] = "telenet-1C060";     //  your network SSID (name)
char pass[] = "bureau15";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient client;

int hour = 18;
int minute = 45;
int day = 16;
int month = 1;
int year = 2017;

int ahours = 18;
int aminute = 45;
int aday = 16;
int amonth = 1;
int ayear = 2017;

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

void setup() {
  Serial.begin(9600);

  pinMode(offPin, INPUT_PULLUP);//initialize certain pins
  pinMode(spkrPin, OUTPUT);
  pinMode(menuPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  dht.begin();

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // For debugging, wait until the serial console is connected
  delay(4000);
  while (!Serial);

  int wifiStatus = WL_IDLE_STATUS;

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  Serial.println("OK");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    delay(5000);
  }

  Serial.println("Setup complete.\n");

  lcd.clear();
  lcd.createChar(0, arrow);
  randomSeed(millis());
}

void loop() {
  dispTime(0);

  /*delay(50);
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
    }*/
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


void dispTime(boolean time) {

  DateTime now = rtc.now();

  /*int hours = now.hour();
    int minutes = now.minute();

    lcd.setCursor(1, 0);
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
    }*/

  if (now.hour() < 10) {
    Serial.print("0");
    Serial.print(now.hour());
  } else {
    Serial.print(now.hour());
  }

  Serial.print(":");
  if (now.minute() < 10) {
    Serial.print("0");
    Serial.print(now.minute());
    Serial.print("\t");
  } else {
    Serial.print(now.minute());
    Serial.print("\t");
  }

  //checkAlarms();
  dispTemp();
  dispDay();
}

void dispDay() { //print date in d.m.y on second line

  DateTime now = rtc.now();

  /*lcd.setCursor(2, 1);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(" ");
    lcd.print(now.day());
    lcd.print("/");
    lcd.print(now.month());
    lcd.print("/");
    lcd.print(now.year());
    lcd.print("    ");*/

  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(" ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.year());
  Serial.print("   ");
}

void dispTemp() {

  // Read temperature as Celsius
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  /*lcd.setCursor(9, 0);
    lcd.print(t);
    lcd.print("C");*/

  Serial.print(t);
  Serial.print("*C\n");
}


void checkAlarms() {

  if (hour == ahours && minute == aminute && day == aday && month == amonth && year == ayear) {

    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Running SendEmail ");

    TembooChoreo SendEmailChoreo(client);

    // Invoke the Temboo client
    SendEmailChoreo.begin();

    // Set Temboo account credentials
    SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
    SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set Choreo inputs
    String UsernameValue = "alarmclocklottev@gmail.com";
    SendEmailChoreo.addInput("Username", UsernameValue);
    String ToAddressValue = "alarmclocklottev@gmail.com";
    SendEmailChoreo.addInput("ToAddress", ToAddressValue);
    String SubjectValue = "AlarmClock";
    SendEmailChoreo.addInput("Subject", SubjectValue);
    String MessageBodyValue = "Wake up!!! Wake up!!! Wake up!!!";
    SendEmailChoreo.addInput("MessageBody", MessageBodyValue);
    String PasswordValue = "jeiiavdlzjgyfkrq";
    SendEmailChoreo.addInput("Password", PasswordValue);

    // Identify the Choreo to run
    SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");

    // Run the Choreo; when results are available, print them to serial
    SendEmailChoreo.run();

    while (SendEmailChoreo.available()) {
      char c = SendEmailChoreo.read();
      Serial.print(c);
    }
    SendEmailChoreo.close();
  }

  Serial.println("\nChecking Alarm\n");
  delay(30000); // wait 30 seconds between SendEmail calls

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
