#include <Wire.h>
#include "MS5837.h"
#include "TSYS01.h"

#include <SPI.h>

#include <Ethernet2.h>
#include <EasyWebServer.h>    // Must be included after Ethernet2.h

#include <SD.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Use this static IP if DHCP fails to assign
IPAddress ip(192, 168, 17, 20);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;

// default for 32u4 and m0
#define WIZ_CS 10

// SD card initialization
// Set the pins used
#define SD_CS 4

Sd2Card card;
SdVolume volume;
SdFile root;

#define USE_PRESSURE
#define USE_TEMPERATURE

#ifdef USE_PRESSURE
MS5837 pressure;
#endif

#ifdef USE_TEMPERATURE
TSYS01 temperature;
#endif

char flash = HIGH;

void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial) { ; }

  Serial.println("Initializing Wire...");
  Wire.begin();

#ifdef USE_TEMPERATURE
    Serial.println("Initializing temperature sensor...");
  temperature.init();
#endif

#ifdef USE_PRESSURE
  Serial.println("Initializing pressure sensor...");
  pressure.init();
  
  pressure.setFluidDensity(1029); // kg/m^3 (997 freshwater, 1029 for seawater)
#endif


  // see if the card is present and can be initialized:
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("Failed to initialize SD card");
    //error(2);
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    
    // Use the pre-configured address instead
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  Serial.println("Finished with initialization...");

}

void loop() {

  digitalWrite(LED_BUILTIN, flash);   // turn the LED on (HIGH is the voltage level)
  flash = (flash == HIGH) ? LOW : HIGH;

  queryI2CSensors();
  
  checkHTTPServer();

      Serial.println("--- --- ---");
   delay(1000);                       // wait for a second

}

void queryI2CSensors()
{
  #ifdef USE_TEMPERATURE
  temperature.read();
  Serial.print("Temperature: ");
  Serial.print(temperature.temperature()); 
  Serial.println(" deg C");
#endif

#ifdef USE_PRESSURE
  pressure.read();

  Serial.print("Pressure: "); 
  Serial.print(pressure.pressure()); 
  Serial.println(" mbar");

  Serial.print("Temperature (from pressure sensor): "); 
  Serial.print(pressure.temperature()); 
  Serial.println(" deg C");
  
  Serial.print("Depth: "); 
  Serial.print(pressure.depth()); 
  Serial.println(" m");
  
  Serial.print("Altitude: "); 
  Serial.print(pressure.altitude()); 
  Serial.println(" m above mean sea level");
#endif
}

void checkHTTPServer()
{
   client = server.available();
  if (client) {
  EasyWebServer w(client);
  w.serveUrl("/",rootPage);
  }
}

void rootPage(EasyWebServer &w){
  w.client.println(F("<!DOCTYPE HTML>"));
  w.client.println(F("<html><head><title>EasyWebServer</title></head><body>"));
  w.client.println(F("<p>Welcome to my little web server.</p>"));
  w.client.println(F("<p><a href='/analog'>Click here to see the analog sensors</a></p>"));
  w.client.println(F("<p><a href='/digital'>Click here to see the digital sensors</a></p>"));
  w.client.println(F("</body></html>"));
}
