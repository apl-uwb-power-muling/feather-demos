#include <Wire.h>
#include "MS5837.h"
#include "TSYS01.h"

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

  Serial.println("Finished with initialization...");

}

void loop() {

  digitalWrite(LED_BUILTIN, flash);   // turn the LED on (HIGH is the voltage level)
  flash = (flash == HIGH) ? LOW : HIGH;

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

  Serial.println("--- --- ---");
    delay(1000);                       // wait for a second

}
