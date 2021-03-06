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





long timeSinceLastBroadcast;

int currentTemp = 50; // TODO poll the sensor at the beginning to get the current temp
int setTemp = 75;
bool heaterIsOn = false;

// codes for sending messsages
const char STATUS_CODE = 's';
const char TEMP_VALUE = 't';

// status code states
const char STATUS_OFF = '0';
const char STATUS_ON  = '1';
const char STATUS_DISCONNECTED = '2';
const char STATUS_CONNECTED = '3';

// codes for recieving messages
const char CHANGE_TEMP  = 'c';
const char HEATER_PWR   = 'p';

// codes for recieving power states
const char HEATER_ON  = '1';
const char HEATER_OFF = '0';





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
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit UART Controller"));
  Serial.println(F("---------------------------------------"));

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

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }

  timeSinceLastBroadcast = millis();
  broadcastCurrTemp();
}


void sendBtMessage(String msg) {
  Serial.print("[Send]: ");
  Serial.println(msg);

  ble.print("AT+BLEUARTTX=");
  ble.println(msg);

  // check response stastus
  if (! ble.waitForOK() ) {
    Serial.println(F("Failed to send?"));
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

  delay(100);
}

void parseMessage(char* msg) {
  if (msg[0] == STATUS_CODE) {
    Serial.print("STATUS CODE: ");

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

        if (heaterIsOn) {
          sendStatusMessage(STATUS_ON);
        } else {
          sendStatusMessage(STATUS_OFF);
        }
        
        break;
      default:
        Serial.println("UNKNOWN CODE");
        break;
    }
  } else if (msg[0] == HEATER_PWR) {
    Serial.print("HEATER POWER: ");
    if (msg[2] == HEATER_ON) {
      Serial.println("ON");
      sendStatusMessage(STATUS_ON);
      heaterIsOn = true;
    } else if (msg[2] == HEATER_OFF) {
      Serial.println("OFF");
      sendStatusMessage(STATUS_OFF);
      heaterIsOn = false;
    } else {
      Serial.println("UNKNOWN CODE");
    }
  } else if (msg[0] == CHANGE_TEMP) {
    int numLen = strlen(msg) - 2;

    char subText[numLen + 1];
    memcpy(subText, &msg[2], numLen);
    subText[numLen] = '\0';

    int parsedVal = atoi(subText);

    Serial.print("CHANGING SET TEMP TO: ");
    Serial.println(parsedVal);

    setTemp = parsedVal;
  }
}

/**************************************************************************/
/*!
    @brief  Checks for user input (via the Serial Monitor)
*/
/**************************************************************************/
bool getUserInput(char buffer[], uint8_t maxSize) {
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while ( (!Serial.available()) && !timeout.expired() ) {
    delay(1);
  }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count = 0;
  do
  {
    count += Serial.readBytes(buffer + count, maxSize);
    delay(2);
  } while ( (count < maxSize) && (Serial.available()) );

  return true;
}

void broadcastCurrTemp(long interval) {
  if (millis() - timeSinceLastBroadcast > interval) {
    broadcastCurrTemp();
    timeSinceLastBroadcast = millis();
  }
}

void broadcastCurrTemp() {
  Serial.print("BROADCASTING CURRENT TEMP: ");
  Serial.println(currentTemp);
  String msg = String(TEMP_VALUE);
  msg = msg + " " + currentTemp;
  sendBtMessage(msg);


  // !!!IMPORTANT!!! REMOVE THIS CODE WHEN ACTUALLY POLLING THE SENSOR FOR TEMP
  // CODE IS HERE FOR DEMO PURPOSES
  if (heaterIsOn && currentTemp > setTemp) {
    currentTemp--;
  } else if (heaterIsOn && currentTemp < setTemp) {
    currentTemp++;
  }
}

void loop(void) {
  char* msg = pollBtMessages();

  // if our message is not null
  if (strcmp(msg, "") != 0) {
    parseMessage(msg);
  }

  broadcastCurrTemp(5000);
}
