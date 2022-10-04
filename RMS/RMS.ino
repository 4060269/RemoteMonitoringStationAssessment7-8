/*
    Arduino-based Remote Monitoring System (RMS) Solution
    for 'Project X' Canberra Floating Hotel.

    Functionality:
    Webserver via HUZZAH32 Feather Board
    - AsyncTCP & ESPAsyncWebServer
    Real-time logging via Adalogger Featherwing
    - RTClib
    Automatic Fan Subsystem via ADXL343 + ADT7410 FeatherWing | DC Motor + Stepper FeatherWing
    -  Adafruit ADT7410 Library & Adafruit Motor Shield V2 Library
    Window Blind Control Subsystem via Mini TFT with Joystick Featherwing | Micro Servo SG90
    - Adafruit ST7735 and ST7789 Library & ESP32Servo
    Safe Security Subsystem via RFID Reader MFRC522 | Red & Green LEDS
    - MFRC522

    Libraries:
    AsyncTCP (Latest as of 17/10/19) by me-no-dev
    ESPAsyncWebServer (Latest as of 26/3/22) by me-no-dev
    RTClib (2.1.1) by Adafruit
    Adafruit ADT7410 Library (1.2.0) by Adafruit
    Adafruit Motor Shield V2 Library (1.1.0) by Adafruit
    Adafruit ST7735 and ST7789 Library (1.9.3) by Adafruit
    ESP32Servo (0.11.0) by Kevin Harrington
    MFRC522 (1.4.10) by GithubCommunity
*/

// Miscellaneous START
#include <Wire.h>
// Providing library to allow communication to other modules via SDA and SCL
#define FORMAT_SPIFFS_IF_FAILED true
// Simplify true to natural language
#define LOOPDELAY 100
// Simplify 100ms to natural language
// Miscellaneous END

// Built In LED START
boolean LEDOn = false;
// Simplify false to natural language
// Built In LED END

// WiFi & Webserver START
#include "sensitiveInformation.h"
// Provide WiFi and Webserver with SSID, usernames and passwords
#include "WiFi.h"
// Providing library to allow software to recognize and communicate with ESP32 WiFi on SoC
#include "SPIFFS.h"
// Provides library to allow software to access the flash on ESP32 using a simple file system
#include <AsyncTCP.h>
// Allows the ESP32 MCUs to create asynchronous and multiple TCP connections
#include <ESPAsyncWebServer.h>
// Allows for the ESP32 to become a webserver using Async HTTP and WebSockets
AsyncWebServer server(80);
// Starting the object to allow other code to run and putting on port 80 for simplistically
// WiFi & Webserver END

// MiniTFT START
#include <Adafruit_GFX.h>
// Core graphics library needed for all Adafruit displays
#include <Adafruit_ST7735.h>
// Hardware-specific library for ST7735 TFT
#include "Adafruit_miniTFTWing.h"
// Library to provide functionaity to seesaw converter framework
Adafruit_miniTFTWing ss;
// Shorten object name to ss for simplistically
#define TFT_RST  -1
// We use the seesaw for resetting to save a pin
#define TFT_CS   14
// Simplify pin 14 to TFT_CS
#define TFT_DC   32
// Simplify pin 32 to TFT_DC
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
// Create TFT object and pass pin definitions for functionality
// MiniTFT END

// Temperature START
#include "Adafruit_ADT7410.h"
// Library to allow temperature sensor to be found and read from software
Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();
// Create the ADT7410 temperature sensor object
// Temperature END

// RTC START
#include "RTClib.h"
// Library for real-time clock to be read from through software
RTC_PCF8523 rtc;
// Initialize object with the name rtc for simplistically
// RTC END

// MotorShield START
#include <Adafruit_MotorShield.h>
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(4);
// Initialize DC Motor object and pass the pin for functionailty
// MotorShield END

// Servo START
#include <ESP32Servo.h>
Servo myservo; 
// Initialize servo object to control a servo
int servoPin = 12;
boolean blindsOpen = false;
// Servo END

// RFID Start
#define LEDRed 27
#define LEDGreen 33
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN  21
#define RST_PIN 17
// Simplifying pin definitions
MFRC522 rfid(SS_PIN, RST_PIN);
bool safeLocked = true;
// RFID End


void setup() {
  // Miscellaneous START
  Serial.begin(9600);
  // 9600 for consistency 
  while (!Serial) {
    delay(10);
    // Keep waiting until serial connection is established 
  }
  delay(1000);

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    // Follow instructions in README and install
    // https://github.com/me-no-dev/arduino-esp32fs-plugin
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  if (!ss.begin()) {
    Serial.println("Seesaw init error!");
    while (1);
    // Waiting for seesaw to start
  }
  else Serial.println("Seesaw started");
  // Miscellaneous END

  // MiniTFT START
  ss.tftReset();
  ss.setBacklight(0x0);
  // reset and turn display on to clear all previous uses
  tft.initR(INITR_MINI160x80);
  // initialize a ST7735S chip, mini display
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  // MiniTFT END


  // Built In LED START
  pinMode(LED_BUILTIN, OUTPUT);
  // Built In LED END


  // WiFi & Webserver START
  WiFi.begin(ssid, password);
  //
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    // Waiting for WiFi connection
  }
  Serial.println();
  Serial.print("Connected to the Internet");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Print details after successful connection

  routesConfiguration();
  // Run function to define routes before server starts to avoid errors
  server.begin();
  // WiFi & Webserver END


  // Temperature START
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find ADT7410!");
    while (1);
    // Waiting for temperature sensor start
  }
  delay(250);
  // Temp sensor takes 250 ms to start reading once started 
  // Temperature END


  // RTC START
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    // Flush to ensure that is not the issue
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    //
    logEvent("RTC is NOT initialized, let's set the time!");
    //
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //
  }
  rtc.start(); //
  // RTC END


  // MotorShield START
  AFMS.begin();
  //
  // MotorShield END


  // Servo START
  ESP32PWM::allocateTimer(0);
  //
  ESP32PWM::allocateTimer(1);
  //
  ESP32PWM::allocateTimer(2);
  //
  ESP32PWM::allocateTimer(3);
  //
  myservo.setPeriodHertz(50);
  // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000);
  // attaches the servo on pin 18 to the servo object
  // Servo END


  // RFID Start
  SPI.begin();
  // initialize SPI bus
  rfid.PCD_Init();
  // initialize MFRC522
  pinMode(LEDRed, OUTPUT);
  //
  pinMode(LEDGreen, OUTPUT);
  //
  digitalWrite(LEDRed, LOW);
  //
  digitalWrite(LEDGreen, LOW);
  //
  // RFID End

}

void loop() {
  builtinLED();
  // Running builtinLED function
  printTemperature();
  // Running printTemperature function
  automaticFan(18.3);
  // Running automaticFan function; passing arguement, < 20.3 degrees will trigger the fan
  windowShutters();
  // Running windowShutters function
  readRFID();
  // Running readRFID function
  safeStatusDisplay();
  // Running safeStatusDisplay function
  delay(LOOPDELAY);
  // To allow time to publish new code.
}


void logEvent(String dataToLog) { //
  // Log entries to a file stored in SPIFFS partition on the ESP32.
  // Get the updated/current time
  DateTime rightNow = rtc.now();
  char csvReadableDate[25];
  sprintf(csvReadableDate, "%02d,%02d,%02d,%02d,%02d,%02d,",  rightNow.year(), rightNow.month(), rightNow.day(), rightNow.hour(), rightNow.minute(), rightNow.second());

  String logTemp = csvReadableDate + dataToLog + "\n";
  // Add the data to log onto the end of the date/time

  const char * logEntry = logTemp.c_str();
  // convert the logtemp to a char * variable

  // Add the log entry to the end of logevents.csv
  appendFile(SPIFFS, "/logEvents.csv", logEntry);

  // Output the logEvents - FOR DEBUG ONLY. Comment out to avoid spamming the serial monitor.
  // readFile(SPIFFS, "/logEvents.csv");

  Serial.print("\nEvent Logged: ");
  Serial.println(logEntry);
}


void builtinLED() { //
  if (LEDOn) {
    //
    digitalWrite(LED_BUILTIN, HIGH);
    //
  } else {
    //
    digitalWrite(LED_BUILTIN, LOW);
    //
  }
}


void tftDrawText(String text, uint16_t color) {
  //
  tft.fillScreen(ST77XX_BLACK);
  //
  tft.setCursor(0, 0);
  //
  tft.setTextSize(3);
  //
  tft.setTextColor(color);
  //
  tft.setTextWrap(true);
  //
  tft.print(text);
  //
}


void printTemperature() {
  //
  float c = tempsensor.readTempC();
  //
  String tempInC = String(c);
  //
  tftDrawText(tempInC, ST77XX_WHITE);
  //
  delay(100);
  //
}


void automaticFan(float temperatureThreshold) {
  //
  float c = tempsensor.readTempC();
  //
  myMotor->setSpeed(100);
  //
  if (c < temperatureThreshold) {
    //
    myMotor->run(RELEASE);
    //
  } else {
    //
    myMotor->run(FORWARD);
    //
  }
}


void windowShutters() {
  //
  uint32_t buttons = ss.readButtons();
  //
  if (! (buttons % TFTWING_BUTTON_A)) {
    //
    if (blindsOpen) {
      myservo.write(0);
      //
    } else {
      //
      myservo.write(180);
      //
    }
    blindsOpen = !blindsOpen;
    //
  }
}


void readRFID() { //
  String uidOfCardRead = "";
  //
  String validCardUID = "60 135 43 73";
  //

  if (rfid.PICC_IsNewCardPresent()) {
    // new tag is available
    if (rfid.PICC_ReadCardSerial()) {
      // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      //
      for (int i = 0; i < rfid.uid.size; i++) {
        //
        uidOfCardRead += rfid.uid.uidByte[i] < 0x10 ? " 0" : " ";
        //
        uidOfCardRead += rfid.uid.uidByte[i];
        //
      }
      Serial.println(uidOfCardRead);
      //

      rfid.PICC_HaltA();
      // halt PICC
      rfid.PCD_StopCrypto1();
      // stop encryption on PCD
      uidOfCardRead.trim();
      //
      if (uidOfCardRead == validCardUID) {
        //
        safeLocked = false;
        //
        logEvent("Safe Unlocked");
        //
      } else {
        safeLocked = true;
        //
        logEvent("Safe Locked");
        //
      }
    }
  }
}


void safeStatusDisplay() {
  //
  /*
     Outputs the status of the Safe Lock to the LEDS
     Red LED = Locked
     Green LED = Unlocked.
  */
  if (safeLocked) { //
    digitalWrite(LEDRed, HIGH);
    //
    digitalWrite(LEDGreen, LOW);
    //
  } else {
    digitalWrite(LEDRed, LOW);
    //
    digitalWrite(LEDGreen, HIGH);
    //
  }
}
