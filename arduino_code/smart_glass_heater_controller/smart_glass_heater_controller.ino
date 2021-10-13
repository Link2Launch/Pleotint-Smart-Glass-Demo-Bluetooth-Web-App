/*********************************************************************
  This is an example for our nRF51822 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

      FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
     
                                Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                                running this at least once is a good idea.
     
                                When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                                Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
         
                                Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines
/*
  SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

  Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


/*********************************************/
// THERMISTOR CODE

// which analog pin to connect
#define THERMISTOR1PIN A1
#define THERMISTOR2PIN A2
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 1000

int samples[NUMSAMPLES];

long timeSinceLastTempPoll;


/**********************************************/


long timeSinceLastBroadcast;

int probe1Temp;
int probe2Temp;
int setTemp1 = 75;
int setTemp2 = 75;
const int heatActvThresh = 0; // Heater activation threshold (in degrees F)

const int maxTemp = 135; // absolute maximum temperature the heater will ever go

bool heater1SwitchIsOn = false;
bool heater2SwitchIsOn = false;
bool heater1IsOn = false;
bool heater2IsOn = false;

// codes for sending messsages
const char STATUS_CODE = 's';
const char TEMP1_VALUE = 't';
const char TEMP2_VALUE = 'k';

// status code states
const char STATUS_OFF =    '0'; // status message, bt heater switch off
const char STATUS_ON  =    '1'; // status message, bt heater switch on
const char STATUS_H1_OFF = '2'; // status message, heater 1 off
const char STATUS_H1_ON  = '3'; // status message, heater 1 on
const char STATUS_H2_OFF = '4'; // status message, heater 2 off
const char STATUS_H2_ON  = '5'; // status message, heater 2 on
const char STATUS_MAX_TEMP =     '6'; // status message, heater reached max temp, turned off for safety
const char STATUS_DISCONNECTED = '7'; // status message, bluetooth disconnected
const char STATUS_CONNECTED =    '8'; // status message, bluetooth connected

// codes for recieving messages
const char CHANGE_TEMP1  = 'c';
const char CHANGE_TEMP2  = 'd';
const char HEATER_PWR1   = 'p';
const char HEATER_PWR2   = 'q';

// codes for recieving power states
const char HEATER_ON  = '1';
const char HEATER_OFF = '0';


// Relay pins for the heaters
#define HEATER1RELAY 13
#define HEATER2RELAY 12

#define HEATER1LED 10
#define HEATER2LED 9

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}



/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Link2Launch - Pleotint Smart Glass Heater"));
  Serial.println(F("---------------------------------------"));

  analogReference(EXTERNAL); // for the thermistor

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE ) {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'Pleotint Smart Glass Heater': "));

  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Pleotint Smart Glass Heater")) ) {
    error(F("Could not set device name?"));
  }

  ble.verbose(false);  // debug info is a little annoying after this point!

  timeSinceLastBroadcast = millis();

  timeSinceLastTempPoll = millis();
  pollThermistors(0);

  pinMode(HEATER1RELAY, OUTPUT);
  pinMode(HEATER2RELAY, OUTPUT);
}


void sendBtMessage(String msg) {
  Serial.print("[Send]: ");
  Serial.println(msg);

  ble.print("AT+BLEUARTTX=");
  ble.println(msg);

  // check response status
  if (! ble.waitForOK() ) {
    Serial.println(F("Failed to send? Turning heaters off for safety"));

    heater1SwitchIsOn = false;
    heater2SwitchIsOn = false;
  }
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
char* pollBtMessages() {
  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return "";
  }

  // Some data was found, its in the buffer
  Serial.print(F("[Recv]: "));
  Serial.println(ble.buffer);
  ble.waitForOK();

  return ble.buffer;
}


void sendStatusMessage(char CODE) {
  String msg = String(STATUS_CODE);
  msg = msg + " " + CODE;
  sendBtMessage(msg);

  delay(500);
}

void parseMessage(char* msg) {
  if (msg[0] == STATUS_CODE) {
    Serial.print("[STATUS CODE]: ");

    switch (msg[2]) {
      case STATUS_OFF:
        Serial.println("OFF");
        break;
      case STATUS_ON:
        Serial.println("ON");
        break;
      case STATUS_DISCONNECTED:
        Serial.println("DISCONNECTED");
        sendStatusMessage(STATUS_DISCONNECTED);
        break;
      case STATUS_CONNECTED:
        Serial.println("CONNECTED");
        sendStatusMessage(STATUS_CONNECTED);

        broadcastCurrentHeaterState();

        break;
      default:
        Serial.println("[UNKNOWN CODE]");
        break;
    }
  } else if (msg[0] == HEATER_PWR1) {
    Serial.print("[HEATER SWITCH 1]: ");
    if (msg[2] == HEATER_ON) {
      Serial.println("ON");
      heater1SwitchIsOn = true;
    } else if (msg[2] == HEATER_OFF) {
      Serial.println("OFF");
      heater1SwitchIsOn = false;
    } else {
      Serial.println("UNKNOWN VALUE");
    }

    broadcastCurrentHeaterState();
  } else if (msg[0] == HEATER_PWR2) {
    Serial.print("[HEATER SWITCH 2]: ");
    if (msg[2] == HEATER_ON) {
      Serial.println("ON");
      heater2SwitchIsOn = true;
    } else if (msg[2] == HEATER_OFF) {
      Serial.println("OFF");
      heater2SwitchIsOn = false;
    } else {
      Serial.println("UNKNOWN VALUE");
    }

    broadcastCurrentHeaterState();
  } else if (msg[0] == CHANGE_TEMP1 || msg[0] == CHANGE_TEMP2) {
    int numLen = strlen(msg) - 2;

    char subText[numLen + 1];
    memcpy(subText, &msg[2], numLen);
    subText[numLen] = '\0';

    int parsedVal = atoi(subText);

    if (msg[0] == CHANGE_TEMP1) {
      Serial.print("[CHANGING SET TEMP 1 TO]: ");
      Serial.println(parsedVal);

      setTemp1 = parsedVal;
    } else if (msg[0] == CHANGE_TEMP2) {
      Serial.print("[CHANGING SET TEMP 2 TO]: ");
      Serial.println(parsedVal);

      setTemp2 = parsedVal;
    }
  }
}

void broadcastCurrTemp(long interval) {
  if (millis() - timeSinceLastBroadcast > interval) {
    broadcastCurrTemp();
    timeSinceLastBroadcast = millis();
  }
}

void broadcastCurrTemp() {
  Serial.print("[BROADCASTING TEMP PROBE 1]: ");
  Serial.println(probe1Temp);
  String msg = String(TEMP1_VALUE);
  msg = msg + " " + probe1Temp;
  sendBtMessage(msg);

  delay(100);

  Serial.print("[BROADCASTING TEMP PROBE 2]: ");
  Serial.println(probe2Temp);
  msg = String(TEMP2_VALUE);
  msg = msg + " " + probe2Temp;
  sendBtMessage(msg);
}

void pollThermistors(long interval) {
  if (millis() - timeSinceLastTempPoll > interval) {
    probe1Temp = pollThermistor(THERMISTOR1PIN);
    probe2Temp = pollThermistor(THERMISTOR2PIN);

    timeSinceLastTempPoll = millis();
  }
}

float pollThermistor(uint8_t thermPin) {
  uint8_t i;
  float average;

  // take N samples in a row, with a slight delay
  for (i = 0; i < NUMSAMPLES; i++) {
    samples[i] = analogRead(thermPin);
    delay(10);
  }

  // average all the samples out
  average = 0;
  for (i = 0; i < NUMSAMPLES; i++) {
    average += samples[i];
  }
  average /= NUMSAMPLES;

  //  Serial.print("Average analog reading ");
  //  Serial.println(average);

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //  Serial.print("Thermistor resistance ");
  //  Serial.println(average);

  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C

  float fahrenheit = (steinhart * 9 / 5) + 32;

  Serial.print("[THERMISTOR ");
  Serial.print(thermPin);
  Serial.print(" READING]: ");
  Serial.print(fahrenheit);
  Serial.println(" *F");

  return fahrenheit;
}

void updateHeaterStatus() {
  
  if (probe1Temp >= maxTemp || probe2Temp >= maxTemp) {
    // protection against over heating the heater
    turnOffHeater(HEATER1RELAY);
    turnOffHeater(HEATER2RELAY);

    Serial.println("[HEATER POWER]: OFF FOR SAFETY - EXCEEDED MAX TEMP");
    sendStatusMessage(STATUS_MAX_TEMP);
  } else {
    // heater hasnt exceded max temp

    // check for heater number 1
    if (heater1SwitchIsOn) {
      if (probe1Temp < setTemp1 + heatActvThresh) {
        // if heater switch is on and we haven't exceeded the set temp
        if (turnOnHeater(HEATER1RELAY)) {
          Serial.println("[HEATER 1 POWER]: ON");
          sendStatusMessage(STATUS_H1_ON);
          heater1IsOn = true;
        }
      } else if (probe1Temp > setTemp1 - heatActvThresh) {
        if (turnOffHeater(HEATER1RELAY)) {
          Serial.println("[HEATER 1 POWER]: OFF");
          sendStatusMessage(STATUS_H1_OFF);
          heater1IsOn = false;
        }
      }

      turnOnLED(HEATER1LED);
    } else {
      heater1IsOn = false;

      if (turnOffHeater(HEATER1RELAY)) {
        Serial.println("[HEATER 1 POWER]: OFF");
        sendStatusMessage(STATUS_H1_OFF);
      }

      turnOffLED(HEATER1LED);
    }

    // check for heater number 2
    if (heater2SwitchIsOn) {
      if (probe2Temp < setTemp2 + heatActvThresh) {
        // if heater switch is on and we haven't exceeded the set temp
        if (turnOnHeater(HEATER2RELAY)) {
          Serial.println("[HEATER 2 POWER]: ON");
          sendStatusMessage(STATUS_H2_ON);
          heater2IsOn = true;
        }
      } else if (probe2Temp > setTemp2 - heatActvThresh) {
        if (turnOffHeater(HEATER2RELAY)) {
          Serial.println("[HEATER 2 POWER]: OFF");
          sendStatusMessage(STATUS_H2_OFF);
          heater2IsOn = false;
        }
      }

      turnOnLED(HEATER2LED);
    } else {
      heater2IsOn = false;

      if (turnOffHeater(HEATER2RELAY)) {
        Serial.println("[HEATER 2 POWER]: OFF");
        sendStatusMessage(STATUS_H2_OFF);
      }

      turnOffLED(HEATER2LED);
    }
    
  }
}

void broadcastCurrentHeaterState() {
  if (heater1IsOn) {
    Serial.println("[Broadcast Heater 1 State]: ON");
    sendStatusMessage(STATUS_H1_ON);
  } else {
    Serial.println("[Broadcast Heater 1 State]: OFF");
    sendStatusMessage(STATUS_H1_OFF);
  }

  if (heater2IsOn) {
    Serial.println("[Broadcast Heater 2 State]: ON");
    sendStatusMessage(STATUS_H2_ON);
  } else {
    Serial.println("[Broadcast Heater 2 State]: OFF");
    sendStatusMessage(STATUS_H2_OFF);
  }
}

// RETURNS TRUE IF THE STATE CHANGED TO OFF
bool turnOnHeater(uint8_t heater) {
  return triggerIO(true, heater);
}

// RETURNS TRUE IF THE STATE CHANGED TO ON
bool turnOffHeater(uint8_t heater) {
  return triggerIO(false, heater);
}

bool turnOnLED(uint8_t ledPin) {
  return triggerIO(true, ledPin);
}

bool turnOffLED(uint8_t ledPin) {
  return triggerIO(false, ledPin);
}

void shutOffDevice() {
  heater1SwitchIsOn = false;
  heater2SwitchIsOn = false;
  heater1IsOn = false;
  heater2IsOn = false;

  turnOffHeater(HEATER1RELAY);
  turnOffHeater(HEATER2RELAY);

  turnOffLED(HEATER1LED);
  turnOffLED(HEATER2LED);
}

// RETURNS TRUE IF THE STATE CHANGED
bool triggerIO(int state, uint8_t relay) {
  int currState = digitalRead(relay);

  if (currState == state) {
    return false;
  } else {
    digitalWrite(relay, state);
    return true;
  }
}

void loop(void) {
  pollThermistors(2000);

  if (ble.isConnected()) {

    char* msg = pollBtMessages();

    // if our message is not null
    if (strcmp(msg, "") != 0) {
      parseMessage(msg);
    }

    broadcastCurrTemp(5000);
  } else {

    // Bluetooth is disconnected
    // turn off all heaters for safety
    shutOffDevice();
  }

  updateHeaterStatus();
}
