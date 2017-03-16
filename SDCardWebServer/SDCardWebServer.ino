/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 */

#include <SPI.h>
#include <Ethernet2.h>
#include <SD.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
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

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Hello, I am the Ethernet Featherwing!\n");


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
  Serial.println("Web server is ready....");
  
}


void loop() {
  // listen for incoming clients
  client = server.available();
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
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
//          client.println("<!DOCTYPE HTML>");
//          client.println("<html>");
//          // output the value of each analog input pin
//          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
//            int sensorReading = analogRead(analogChannel);
//            client.print("analog input ");
//            client.print(analogChannel);
//            client.print(" is ");
//            client.print(sensorReading);
//            client.println("<br />");
//          }
//          client.println("</html>");
          printCardInfo( );
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

void printCardInfo() {
    // print the type of card
  client.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      client.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      client.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      client.println("SDHC");
      break;
    default:
      client.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    client.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  client.print("\nVolume type is FAT");
  client.println(volume.fatType(), DEC);
  client.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  client.print("Volume size (bytes): ");
  client.println(volumesize);
  client.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  client.println(volumesize);
  client.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  client.println(volumesize);


//  client.println("\nFiles found on the card (name, date and size in bytes): ");
//  root.openRoot(volume);
//
//  // list all files in the card with date and size
//  root.ls(LS_R | LS_DATE | LS_SIZE);
}

