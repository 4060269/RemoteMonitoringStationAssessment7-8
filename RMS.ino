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

    Board Configuration:
    Adafruit HUZZAH32 – ESP32 Feather Board
    Add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json to Additional Boards Manager URLs located in "File, Preferences"
    Install esp32 (2.0.5) by Espressif Systems in "Tools, Board, Boards Manager"
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
bool fanEnabled = false;
// If the fan is on or off.
bool autoFanEnabled = true;
// Automatic or manual control
// MotorShield END

// Servo START
#include <ESP32Servo.h>
Servo myservo;
// Initialize servo object to control a servo
int servoPin = 12;
boolean blindsOpen = false;
// Servo END

// RFID Start
#define LEDRed 33
#define LEDGreen 27
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
  Serial.begin(115200);
  // 115200 since this is the default for ESP32 https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html#:~:text=The%20default%20console%20baud%20rate%20on%20ESP32%20is%20115200.
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
    tftDrawText("Connecting to WiFi..", ST77XX_RED);
    Serial.println("Connecting to WiFi..");
    // Waiting for WiFi connection
  }
  tft.fillScreen(ST77XX_BLACK);
  Serial.println();
  Serial.print("Connected to the Internet");
  Serial.print("IP address: ");
  String ip = WiFi.localIP().toString();
  Serial.println(ip);
  // Print details to serial after successful connection

  // Display IP on TFT
  tft.setCursor(0, 60);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.print(ip);

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
    logEvent("RTC is NOT initialized, let's set the time!");
    // Run logEvent function with the string argument for debugging
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Initial setup time
  }
  rtc.start();
  // RTC END


  // MotorShield START
  AFMS.begin();
  // Initialize AFMS object
  // MotorShield END


  // Servo START
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  // Need to set time for timer class method
  myservo.setPeriodHertz(50);
  // Standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000);
  // Attaches the servo to the servo object for easy functionality
  // Servo END


  // RFID Start
  SPI.begin();
  // Initialize SPI bus
  rfid.PCD_Init();
  // initialize PCD for MFRC522
  pinMode(LEDRed, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  digitalWrite(LEDRed, LOW);
  digitalWrite(LEDGreen, LOW);
  // RFID End

}

void loop() {
  builtinLED();
  // First, run the dummy function
  printTemperature();
  // Then run the temperature print out to display
  if (autoFanEnabled) {
    automaticFan(22.00);
  }
  fanControl();
  // After those functions, the order is irrelevant
  windowShutters();
  readRFID();
  safeStatusDisplay();
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


void builtinLED() {
  if (LEDOn) {
    // Is true
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
// This function is here for debugging and/or if needed as extra functionality


void tftDrawText(String text, uint16_t color) {
  tft.setCursor(0, 0);
  // Centers where all text should be printed for consistency
  tft.setTextSize(3);
  // Only Medium-sized text is needed
  tft.setTextColor(color, ST77XX_BLACK);
  // Pass a colour that goes with the background colour
  tft.setTextWrap(true);
  // To keep all text, even if too long, on screen
  tft.setRotation(3);
  // Depending on setup, may need to rotate
  tft.print(text);
  // Print text onto display
}
// The functionality of the TFT is all setup with the purpose of having other functions pass their values through
// Showing temperature would only require a simple extra function to display it


void printTemperature() {
  float c = tempsensor.readTempC();
  // Need to take raw temp and make a float to cut unnecessary extra numbers
  String tempInC = String(c);
  // Change it to a string to make it displayable on TFT
  tftDrawText(tempInC, ST77XX_WHITE);
  // Pass the text and colour through to the TFT function to display
  delay(100);
}
// This function is a way to display temperature in real-time effectively and will be used to show guests their pod temp


void automaticFan(float temperatureThreshold) {
  // Get the value from loop to have more efficient and faster code
  float currentTemp = tempsensor.readTempC();
  myMotor->setSpeed(100);
  // Change class method with argument 100, to fully turn on fan
  if (currentTemp > temperatureThreshold) {
    // Compare current with threshold
    fanEnabled = true;
    // Stop running because it is cool/cold
  } else {
    fanEnabled = false;
    //logEvent("Temperature triggered fan on");
    // Run because it is warm/hot
  }
}
// This function is one way the pod automatically can be kept cool if the temperature inside gets to be too hot for the guests

void fanControl() {
  if (fanEnabled) {
    myMotor->setSpeed(150);
    myMotor->run(FORWARD);
  } else {
    myMotor->run(RELEASE);
  }
  // Run automatic fan control if the guests have set it or enable manual control
  // This will simply start and stop the motor
}
// This function is other alternative to controlling temperature in the pod, by letting the guests manually do it


void windowShutters() {
  uint32_t buttons = ss.readButtons();
  // Need to group the two buttons on display into one variable, to make a simple if else statement to address each
  if (! (buttons & TFTWING_BUTTON_A)) {
    // Scenario for first button
    if (blindsOpen) {
      myservo.write(0);
      logEvent("Blinds opening");
      // Send 0 degrees to not rotate
      delay(1000);
    } else {
      // Scenario for second button
      myservo.write(180);
      logEvent("Blinds closing");
      // Send 180 degrees to rotate
    }
    blindsOpen = !blindsOpen;
    // Make itself equal its inverse to reset the function
  }
}
// This function easily allows guests to close, open or adjust their window shutters to their liking
// using the built in buttons on the display

void readRFID() {
  String uidOfCardRead = "";
  // Used for showing the current tag ID
  String validCardUID = "60 135 43 73";
  // A static ID that is allowed for use

  if (rfid.PICC_IsNewCardPresent()) {
    // New tag is available
    if (rfid.PICC_ReadCardSerial()) {
      // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      // Check PICC type for compatibility
      for (int i = 0; i < rfid.uid.size; i++) {
        uidOfCardRead += rfid.uid.uidByte[i] < 0x10 ? " 0" : " ";
        uidOfCardRead += rfid.uid.uidByte[i];
        // reading the ID per letter to display
      }
      Serial.println(uidOfCardRead);

      rfid.PICC_HaltA();
      // halt PICC
      rfid.PCD_StopCrypto1();
      // stop encryption on PCD
      uidOfCardRead.trim();
      // Change ID to same format to compare if it is valid
      if (uidOfCardRead == validCardUID) {
        safeLocked = false;
        logEvent("Safe Unlocked");
      } else {
        safeLocked = true;
        logEvent("Safe Locked");
        // Change the global variable state each time the valid ID is tapped to unlock/lock
      }
    }
  }
}
// This function is needed to operate the safe's status, by reading, validating the card presented
// Then only the checked variable to add into a simple function to open and unlock the safe

void safeStatusDisplay() {
  /*
     Outputs the status of the Safe Lock to the LEDS
     Red LED = Locked
     Green LED = Unlocked.
  */
  if (safeLocked) {
    digitalWrite(LEDRed, HIGH);
    digitalWrite(LEDGreen, LOW);
    // Turn the red LED on
  } else {
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDGreen, HIGH);
    // Turn green on to indicate to the guests that it is unlocked
  }
}
// This function is the user-facing function to show the guests if the safe is unlocked or locked
