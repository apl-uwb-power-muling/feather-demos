#include <Wire.h>
#include "MS5837.h"
#include "TSYS01.h"

#include <SPI.h>
#include <Ethernet2.h>

#include <SD.h>

MS5837 depthsensor;
TSYS01 tempsensor;

byte mac[ ] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 13, 42);
EthernetServer server(80);
EthernetClient client;

String fakename = "Nginx/1.8.0 (ATmega328p/Ubuntu 12.04 LTS)"; //装逼参数(伪装服务器）
String res = "", path = "";

#define WIZ_CS 10
#define SD_CS 4

Sd2Card card;
SdVolume volume;
SdFile root;

#define cardSelect 4

// File logfile;

int chipSelect = 4;
File mySensorData;
char name[13];


float temp;
float temp1;
float Pressure;
float altitude;
float depth;
float time11;
long fileNum = 10000;
unsigned long loopMillis = 0l;

const int NUM_ANALOG_IN = 4;
unsigned int analog[NUM_ANALOG_IN]; 

const int VBATT_ANALOG_IN = 0;
const float R1 = 5.6;   //MOhm
const float R2 = 1.15;   //MOhm
const float VBATT_MULT = 3.3 / 1024 * (R1+R2)/R2;
float vbatt;



void incFileNum() { // generate next file name:

  //AMM:   A bit of C voodoo.  Does the same thing _but_ will make the number portion five
  // characters wide and will add 0s to make it 5 characters wide, for example:
  //     dat00010.txt
  //     dat00011.txt
  // ....
  // This makes it easier to put the filenames in order (thought with fileNum=10000, 
  // they'll be in order anyway)
  snprintf( name, 13, "dat%05d.txt", ++fileNum );
  
//  String s = "dat" + String(++fileNum) + ".txt";
//  s.toCharArray(name,13);
}


void setup() {
  
  Serial.begin(115200);
  Serial.println("Starting");

  //Serial.begin(115200);
  int count = 0;
  const int SERIAL_TIMEOUT = 1000;
  while (!Serial && count++ < SERIAL_TIMEOUT ) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Initializing depth and pressure sensors");
  
  Wire.begin();

  depthsensor.init();
  tempsensor.init();
  
  depthsensor.setFluidDensity(1029); // kg/m^3 (997 freshwater, 1029 for seawater)
  
  Serial.println("Initializing SD card...");
  
  SD.begin(chipSelect) ;

  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("Failed to initialize SD card");
  }
  

  //Serial.print("Hello, I am the Ethernet Featherwing!\n");


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

 

  
  /*Serial.begin(115200);
  Serial.println("\r\nAnalog logger test");
  pinMode(13, OUTPUT);


  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2); 
 
    
    
    */

/*

     Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //启动sd
  Serial.print("init sd:");
  if (!SD.begin(4)) {
    Serial.println("failed");
    return;
  }
  Serial.println("done");
  //启动eth
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("ip:");
  Serial.println(Ethernet.localIP());
    */
    
  // Need to call this once to set 'name'
  incFileNum();
  while (SD.exists(name)) incFileNum();
  Serial.println("new file name: " + String(name));
  
  //--------------------------------------------------

  // Put a header in the file
  mySensorData= SD.open(name, FILE_WRITE);
  //mySensorData.println("# elapsed_milliseconds,temp_c,pressure_mbar,temp_from_pressure_c,depth_m,altitude_m");
  
  mySensorData.print("# seconds,temp_c,pressure_mbar,temp_from_pressure_c,depth_m,altitude_m");

  for( int i = 0; i < NUM_ANALOG_IN; ++i ) {
    mySensorData.print(",analog_");
    mySensorData.print(i);
  }
  mySensorData.println();

  mySensorData.close();

}

/*
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

*/
 
void loop() {

  depthsensor.read();
  tempsensor.read();

  for( byte i = 0; i < NUM_ANALOG_IN; ++i ) {
    analog[i] = analogRead(i);
  }

  // Do this expensive calculation once
  vbatt = analog[VBATT_ANALOG_IN] * VBATT_MULT;

/*
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


 
  Serial.print("Temperature: ");
  Serial.print(tempsensor.temperature()); 
  Serial.println(" deg C");
   
  Serial.println("---");

  delay(1000);

*/
  //  writing data to the SD card
  temp=tempsensor.temperature();
  Pressure=depthsensor.pressure();
  temp1=depthsensor.temperature();
  depth=depthsensor.depth();
  altitude=depthsensor.altitude();
  time11=loopMillis/1000.0;
  // Get the milliseconds since the Arduino turned on:
  //   https://www.arduino.cc/en/Reference/Millis
  loopMillis = millis();
  
  Serial.print("< Temperature(temperature sensor): ");
  Serial.print(temp); 
  Serial.println(" deg C");
  Serial.println("--- >");

  Serial.print("< Pressure: "); 
  Serial.print(Pressure); 
  Serial.println(" mbar >");
  
  Serial.print("< Temperature(pressure sensor): "); 
  Serial.print(temp1); 
  Serial.println(" deg C >");
  
  Serial.print("< Depth: "); 
  Serial.print(depth); 
  Serial.println(" m  >");
  
  Serial.print("< Altitude: "); 
  Serial.print(altitude); 
  Serial.println(" m above mean sea level >");

  Serial.print("< Time: "); 
  Serial.print(time11); 
  Serial.println(" seconds (s) >");

  Serial.print("< VBatt: ");
  Serial.print(  vbatt );
  Serial.println(" Volts >");

  Serial.print("Analog values (raw): ");
  for( int i = 0; i < NUM_ANALOG_IN; ++i ) {
    Serial.print( analog[i] );
    Serial.print( " " );
  }
  Serial.println("");

  //write on SD card

  mySensorData= SD.open(name, FILE_WRITE);

  if (mySensorData){

    if( true ) {
      // AMM:   Aaron's version which saves CSV to the SD card.
      // To use this, set above to "if( true ) { ...."
      // otherwise set to "if( false ) { ...."

      //mySensorData.print((unsigned long)loopMillis);
      mySensorData.print(time11);
      mySensorData.print(',');
      mySensorData.print(temp); 
      mySensorData.print(',');
      mySensorData.print(Pressure); 
      mySensorData.print(',');
      mySensorData.print(temp1); 
      mySensorData.print(',');
      mySensorData.print(depth); 
      mySensorData.print(',');
      mySensorData.print(altitude); 

      for( int i = 0; i < NUM_ANALOG_IN; ++i ) {
        mySensorData.print(',');
        mySensorData.print( analog[i] );
      }
      mySensorData.println("");
      
    } else {
  
      mySensorData.print("<Temperature(temperature sensor): ");
      mySensorData.print(temp); 
      mySensorData.println(" deg C");
      mySensorData.println("--->");
  
  
      mySensorData.print("< Pressure: "); 
      mySensorData.print(Pressure); 
      mySensorData.println(" mbar>");
    
      mySensorData.print("< Temperature(pressure sensor): "); 
      mySensorData.print(temp1); 
      mySensorData.println(" deg C>");
    
      mySensorData.print("< Depth: "); 
      mySensorData.print(depth); 
      mySensorData.println(" m>");
    
      mySensorData.print("< Altitude: "); 
      mySensorData.print(altitude); 
      mySensorData.println(" m above mean sea level>");

      mySensorData.print("< Time: "); 
      mySensorData.print(loopMillis/1000.0); 
      mySensorData.println(" seconds (s) >");

      digitalWrite(8,HIGH);
      delay(100);
      digitalWrite(8,LOW);
      delay(100);
    }
    
    mySensorData.close();
  
    
  }
  else {
    Serial.println("unable to store data in SD card");
  }


/*

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

          //depthsensor.read();

          client.print("\\\\\\Pressure: "); 
          client.print(depthsensor.pressure()); 
          client.println(" mbar ////////");
  
          client.print("\\\\\\\\ Temperature: "); 
          client.print(depthsensor.temperature()); 
          client.println(" deg C ////////");
  
          client.print("\\\\ Depth: "); 
          client.print(depthsensor.depth()); 
          client.println(" m//////");
  
          client.print("\\\\\\\\\\ Altitude: "); 
          client.print(depthsensor.altitude()); 
          client.println(" m above mean sea level/////////");

          //delay(1000);

          //tempsensor.read();
 
          client.print("\\\\\\ Temperature: ");
          client.print(tempsensor.temperature()); 
          client.println("deg C///////");
   
          client.println("---");

          //delay(1000);
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


  // One delay for the whole loop
  delay(1000);

*/
///*
 // Another version
 client = server.available();
  if (client) {
    Serial.println("new client");
    
    delay(200);
    res = ""; path = "";  //httpget = 
    int resend = 1;
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        //只读取一行
        if (c != '\n' && resend) {
          res += c;
        } else {
          resend = 0;
        }

        if (c == '\n' && currentLineIsBlank) {
          //判断GET头完整性
          if ((res.indexOf("GET ") != -1) && (res.indexOf(" HTTP") != -1)) {
            //判断是否存在get参数
            if (res.indexOf('?') != -1) {
              //httpget = res.substring(res.indexOf('?') + 1, res.indexOf(" HTTP"));
              path = res.substring(res.indexOf("GET ") + 4, res.indexOf('?'));
            } else {
              path = res.substring(res.indexOf("GET ") + 4, res.indexOf(" HTTP"));
            }
            Serial.println(res);
            //Serial.println("GET:" + httpget);
            Serial.println("path:" + path);
            //delay(100);
            File s = SD.open(path);
            if (s) {
              if( s.isDirectory() ) {
                webprintDirectory(s);
                client.println("<hr>");
                webprintSensors();         
              } else {
                webprintFile(s);
              }
            } else {
              client.println("Error opening " + path + " : " + s);     // AMM, I think this is an error condition, so print an error message
              //s.rewindDirectory();                                   // AMM probably don't need this.
              s.close();
            }

            break;
          }
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
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
 
//*/


  // AMM Put one master delay at the end so we don't record data too fast
  delay(1000);
  
}


void webprintFile(File file) {

    //AMM: Return the content length, too.   Might make it work better when downloading 
    // big files. 
    char httpHeader[64];
    snprintf(httpHeader,64,"200 OK\nContent-Length: %d", file.size());
 
    if (path.indexOf('.') != -1) {
      //获取后缀名
      http_header(httpHeader, path.substring(path.indexOf('.') + 1));
    } else {      
      http_header(httpHeader, "txt");//文件没有后缀名 默认显示文本格式
    }
    //读文件

    // AMM.  Changed this so it reads/writes multiple bytes at a time.  Should be much faster
    const int BUFFER_LENGTH = 256;
    char buffer[BUFFER_LENGTH];
    while (file.available()) {
      int charsRead = file.read(buffer, BUFFER_LENGTH );
      client.write( buffer, charsRead );
    }
    file.close();
    return;//退出
}

void webprintDirectory( File dir ) {

  //判断不是文件
  http_header("200 OK", "htm");
  client.print("<!DOCTYPE HTML><html><body><h1>Tian Rules!</h1><br />PATH:" + path + "<hr />");
  dir.rewindDirectory();//索引回到第一个位置
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      //没有下一个索引了
      client.print("<br />No more files");
      dir.rewindDirectory();
      break;
    }
    if (path == "/") {
      client.print("<br /><a href=\"" + path);
    } else {
      client.print("<br /><a href=\"" + path + "/");
    }
    client.print(entry.name());
    client.print("\"target=\"_self\">");
    client.print(entry.name());
    client.print("</a>");
    if (!entry.isDirectory()) {
      // files have sizes, directories do not
      client.print("&nbsp;&nbsp;");
      client.println(entry.size(), DEC);
    }

    entry.close();
  }
  client.println("<p>" + fakename + "</p></body></html>");
  dir.close();
}

void http_header(String statuscode, String filetype) {
  filetype.toLowerCase();//把后缀名变小写
  client.println("HTTP/1.1 " + statuscode);
  client.print("Content-Type: ");
  //判断文件mime类型
  if (filetype == "htm" || filetype == "html") {
    client.println("text/html");
  }
  if (filetype == "png" || filetype == "jpg" || filetype == "bmp" || filetype == "gif") {
    client.println("image/" + filetype);
  } else {
    client.println("text/plain");
  }
  client.println("Connection: close");
  client.println();
}

void webprintSensors() {

    client.print("<b>Milliseconds:</b> "); 
    client.print(loopMillis); 
    client.println(" msec <br/>");

    client.print("<b>Pressure:</b> "); 
    client.print(depthsensor.pressure()); 
    client.println(" mbar <br/>");

    client.print("<b>Temperature from pressure sensor:</b> "); 
    client.print(depthsensor.temperature()); 
    client.println(" deg C<br/>");

    client.print("<b>Depth:</b> "); 
    client.print(depthsensor.depth()); 
    client.println(" m<br/>");

    client.print("<b>Altitude:</b> "); 
    client.print(depthsensor.altitude()); 
    client.println(" m above mean sea level<br/><br/>");

    client.print("<b>Temperature sensor:</b> ");
    client.print(tempsensor.temperature()); 
    client.println("deg C<br/>");

  client.print("<b>Battery voltage:</b> ");
  client.print( vbatt );
  client.println("<br/>");

    client.print("<b>Raw analog values:</b> " );
    for( int i = 0; i < NUM_ANALOG_IN; ++i ) {
      client.print(analog[i]);
      client.print(' ' );
    }
    client.println("<br/>");

    client.println("<hr/>");
}

