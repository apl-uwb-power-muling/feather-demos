#include <Wire.h>
#include "MS5837.h"
#include "TSYS01.h"

#include <SPI.h>
#include <Ethernet2.h>

#include <SD.h>

MS5837 depthsensor;
TSYS01 tempsensor;

byte mac[ ] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 0, 2);
EthernetServer server(80);
EthernetClient client;
  #define WIZ_CS 10
#define SD_CS 4

Sd2Card card;
SdVolume volume;
SdFile root;

#define cardSelect 4

File logfile;

void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}


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

if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("Failed to initialize SD card");
    //error(2);
  }
  
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

  
  Serial.begin(115200);
  Serial.println("\r\nAnalog logger test");
  pinMode(13, OUTPUT);


  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
}


char filename[15];
  strcpy(filename, "ANALOG00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }


  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to "); 
  Serial.println(filename);

  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  Serial.println("Ready!");
}

uint8_t i=0;

 
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
  digitalWrite(8, HIGH);
  logfile.print("A0 = "); logfile.println(analogRead(0));
  Serial.print("A0 = "); Serial.println(analogRead(0));
  digitalWrite(8, LOW);
  
  delay(100);

  
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


  client.println("\nFiles found on the card (name, date and size in bytes): ");
  File root = SD.open("/");
  printDirectory(root,0);

//  root.openRoot(volume);
//
//  // list all files in the card with date and size
//  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void printDirectory(File dir, int numTabs)
{
  while (true)
  {
    File entry = dir.openNextFile();
    if (! entry)
    {
      if (numTabs == 0)
        client.println("** Done **");
      return;
    }
    for (uint8_t i = 0; i < numTabs; i++)
      client.print('\t');
    client.print(entry.name());
    if (entry.isDirectory())
    {
      client.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      client.print("\t\t");
      client.println(entry.size(), DEC);
    }
    entry.close();
  }
}


