
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
// Modified by Venkatesh Prasad for EcoBots.Co
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

#include <Wire.h>
#include "OLED.h"
#include "Adafruit_MCP9808.h"

// Declare OLED display
// display(SDA, SCL);
// SDA and SCL are the GPIO pins of ESP8266 that are connected to respective pins of display.
OLED display(4,5);


/************************ Example Starts Here *******************************/


#define RELAY_PIN 16 //Relay Pin for Heater
#define HYSTERESIS 1 // Deadband (degF) around Set_Temp to avoid relay chatter

// set up the  feed
AdafruitIO_Feed *heatswitch = io.feed("heatswitch"); 
AdafruitIO_Feed *settemp = io.feed("settemp");
AdafruitIO_Feed *roomtemp = io.feed("roomtemp");

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808(); 

int f = 0; // Stores your current Room Temperature in Faranheit 
int lastf=-1;
bool man_set = false; // true while the heatswitch feed is manually overriding the thermostat
int Set_Temp = 0; // 0 until a value arrives on the settemp feed
bool heater_on = false;

void setup() {

  // set led pin as a digital output
  pinMode(RELAY_PIN, OUTPUT);

  // start the serial connection
  Serial.begin(115200); // Serial comminication for Debugging 
  display.begin();
  display.clear();
  display.on();
  
  // wait for serial monitor to open
  while(! Serial);

  
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }

  

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  display.print("Connecting to Adafruit IO");
  io.connect();

  // set up a message handler for the 'digital' feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  heatswitch->onMessage(handleMessage);
  settemp-> onMessage(handleSetTemp);
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    //display.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  display.clear();
  display.print("Connected",1);
  delay(3000);
  display.clear();

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
  float c = tempsensor.readTempC();
  f = c * 9.0 / 5.0 + 32;// Convert degC to degF for us in the US :)

  // Automatic control: heat below Set_Temp - HYSTERESIS, stop above Set_Temp + HYSTERESIS
  if(!man_set && Set_Temp > 0) {
    if(!heater_on && f <= Set_Temp - HYSTERESIS) {
      setHeater(true);
    }
    else if(heater_on && f >= Set_Temp + HYSTERESIS) {
      setHeater(false);
    }
  }

  // return if the value hasn't changed
  if(f == lastf)
    return;

  // save the current state to the analog feed
  Serial.print("sending -> ");
  Serial.println(f);
  roomtemp->save(f);

  // store last photocell state
  lastf = f;

}

void setHeater(bool on) {
  heater_on = on;
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
  Serial.print("Heater ");
  Serial.println(on ? "ON" : "OFF");
}

// this function is called whenever an 'digital' feed message
// is received from Adafruit IO. it was attached to
// the 'digital' feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {
  //display.clear();
  Serial.print("received <- ");
  
  // manual override until a new set temperature is received
  man_set = true;
  setHeater(data->toPinLevel() == HIGH);

  if(heater_on){
    Serial.println("HIGH");
    display.on();
    display.print("Heater ON ",2);
    delay(5000);
    display.off();
    }
  else {
    Serial.println("LOW");
    display.on();
    display.print("Heater OFF",2);
    delay(5000);
    display.off();
    }



}

void handleSetTemp(AdafruitIO_Data *data) {

  // convert the data to integer
  int reading = data->toInt();
  Set_Temp = reading;
  man_set = false; // a new set temperature returns control to the thermostat
  String A=String(reading);
  char Value[5];
  A.toCharArray(Value,5);
  display.on();
  Serial.print("received <- ");
  Serial.println(reading);
  
  display.print("Set Temp.",3);
  display.print(Value,3,12);

  A=String(f);
  A.toCharArray(Value,5);
  
  display.print("Cur. Temp.",4);
  display.print(Value,4,12);
  delay(5000);
  display.clear();
  

}
