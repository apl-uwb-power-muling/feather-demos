#include <Wire.h>
#include "MS5837.h"
#include "TSYS01.h"

#include <SPI.h>
#include <Ethernet2.h>

MS5837 depthsensor;
TSYS01 tempsensor;

byte mac[ ] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 0, 2);
EthernetServer server(80);
 
  #define WIZ_CS 10


void setup() {
  
  Serial.begin(9600);
  
  Serial.println("Starting");
  
  Wire.begin();

  depthsensor.init();
  tempsensor.init();
  
  depthsensor.setFluidDensity(1029); // kg/m^3 (997 freshwater, 1029 for seawater)

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Hello, I am the Ethernet Featherwing!\n");


  // start the Ethernet connection:
  //if (Ethernet.begin(mac) == 0) {
    //Serial.println("Failed to configure Ethernet using DHCP");
    
    // Use the pre-configured address instead
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  //}
  
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("\nWeb server is ready....\n");
  
}
void loop() {

  depthsensor.read();

  Serial.print("Pressure: "); 
  Serial.print(depthsensor.pressure()); 
  Serial.println(" mbar");
  
  Serial.print("Temperature: "); 
  Serial.print(depthsensor.temperature()); 
  Serial.println(" deg C");
  
  Serial.print("Depth: "); 
  Serial.print(depthsensor.depth()); 
  Serial.println(" m");
  
  Serial.print("Altitude: "); 
  Serial.print(depthsensor.altitude()); 
  Serial.println(" m above mean sea level");

  delay(1000);

  tempsensor.read();
 
  Serial.print("Temperature: ");
  Serial.print(tempsensor.temperature()); 
  Serial.println(" deg C");
   
  Serial.println("---");

  delay(1000);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        // Look for HTTP verb (GET) and requested path

        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  }

