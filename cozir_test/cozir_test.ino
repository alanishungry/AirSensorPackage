#include <SPI.h>

//COZIR
const int cozir = 2;
double val;
double CO2conc = 0.0;
double CO2sum = 0.0;
double FS = 2000;
double vsupply;
double percentOfMax;
// pin A6 = analog voltage for COZIR

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A14, INPUT);

//  SPI.setClockDivider(SPI_CLOCK_DIV4);
//  SPI.setBitOrder(MSBFIRST);
//  SPI.setDataMode(SPI_MODE0);
}

void loop() {
  vsupply = 1023;
  val = analogRead(A14);
  CO2conc = FS * (val / vsupply);
//  Serial.print("V Supply = ");
//  Serial.println(vsupply);
//  Serial.print("Analog Voltage = ");
//  Serial.println(val);
  Serial.print("CO2 Concentration = ");
  Serial.println(CO2conc);
//  CO2conc = (val*3.28)/FS;
  delay(1000);
  
//    val = analogRead(cozir);
//    //Serial.println(val);
//     CO2conc = (val*2.25 - 126.00-375); 
//     //val*2.25 - 126.00-375
//    CO2sum=CO2conc+CO2sum;
}
