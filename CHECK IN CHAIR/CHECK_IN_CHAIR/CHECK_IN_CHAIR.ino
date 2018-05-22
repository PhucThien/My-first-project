#include "HX711.h"
#include <SPI.h>
#include <RFID.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"

#define calibration_factor -23016.0 //This value is obtained using the SparkFun_HX711

#define DOUT  4
#define CLK  3
#define SDA_DIO 9
#define RESET_DIO 8
#define ONE_WIRE_BUS 2
#define REPORTING_PERIOD_MS     1000

HX711 scale(DOUT, CLK);
RFID RC522(SDA_DIO, RESET_DIO);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celcius = 0;
char y;
int IDCard1 = 0, IDCard2 = 0, IDCard3 = 0, IDCard4 = 0, IDCard5 = 0;
float Temp = 0,Weight=0;
int SpO2 = 0, HRate = 0;
PulseOximeter pox;

uint32_t tsLastReport = 0;


void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  Serial.begin(38400);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();
  SPI.begin();
  /* Initialise the RFID reader */
  RC522.init();
  sensors.begin();

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  // The default current for the IR LED is 50mA and it could be changed
  // Check MAX30100_Registers.h
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop()
{ 
  if (Serial.available()) {
    char x = Serial.read();
    if (x == 'R') {
      scale.tare();
    }
  }
  
  if (RC522.isCard())
  {
    /* If so then get its serial number */
    RC522.readCardSerial();

    IDCard1 = RC522.serNum[0];
    IDCard2 = RC522.serNum[1];
    IDCard3 = RC522.serNum[2];
    IDCard4 = RC522.serNum[3];
    IDCard5 = RC522.serNum[4];

  }
  Laydulieu();
  delay(1000);
  // Make sure to call update as fast as possible
  if (Serial.available()) {
    char x = Serial.read();
    if (x == 'B') {
      while (1) {
        if (Serial.available()) {
          y = Serial.read();
          if (y == 'S') {
            break;
          }
        }
        sensors.requestTemperatures();
        Celcius = sensors.getTempCByIndex(0);
        Temp = Celcius;
        Weight=scale.get_units(10);
        Laydulieu();
      }
    }
  }

  // Asynchronously dump heart rate and oxidation levels to the serial
  // For both, a value of 0 means "invalid"
  if (Serial.available()) {
    char x = Serial.read();
    if (x == 'A') {
      while (1) {
        if (Serial.available()) {
          y = Serial.read();
          if (y == 'S') {
            break;
          }
        }
        pox.update();
        SpO2 = pox.getSpO2();
        HRate = pox.getHeartRate();
        Laydulieu();
      }
    }
  }
}
void Laydulieu() {
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print(IDCard1);
    Serial.print(IDCard2);
    Serial.print(IDCard3);
    Serial.print(IDCard4);
    Serial.print(IDCard5);
    Serial.print(',');
    Serial.print(Weight);
    Serial.print(',');
    Serial.print(Temp);
    Serial.print(',');
    //Serial.print(SpO2);
    //Serial.print(',');
    Serial.print(HRate);
    Serial.println('E');

    tsLastReport = millis();
  }
}

