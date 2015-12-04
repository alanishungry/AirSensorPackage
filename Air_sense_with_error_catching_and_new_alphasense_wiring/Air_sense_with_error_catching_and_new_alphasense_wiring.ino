/*
 * Bergin Lab - Air Sensor Package
 * Logs box number, date, time, rh, temp, CO2, PM1/2.5/10
 * by Alan Guo
 *
 *Format:
 *yyyy/mm/dd hh:mm:ss tsi_pm1 tsi_pm2.5 tsi_pm10 pm1 pm2.5 pm10 CO2 temp(C) humidity(%)
 *
 */

//WRITE DATA TO FILE BELOW!!
const char* buffer = "text.txt";

/***Libraries***/
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include <SD.h>
#include <SD_t3.h>
#include <SHT1X.h>


//Definitions
#define LOOP_DELAY 200 //ms delay time in loop

/***Global Variables***/
unsigned long timeElapsed;
unsigned long firstTime;
unsigned long logTime;
unsigned long prevTimeElapsed;

int sampleSize;
int redPin = 5;
int yellowPin = 6;
int orangePin = 7;

uint16_t TPM01ValueAvg;
uint16_t TPM2_5ValueAvg;
uint16_t TPM10ValueAvg;
uint16_t PM01ValueAvg;
uint16_t PM2_5ValueAvg;
uint16_t PM10ValueAvg;

double CO2concAvg;

float tempCavg;
float tempFavg;
float humidityAvg;

uint16_t TPM01ValueSum;
uint16_t TPM2_5ValueSum;
uint16_t TPM10ValueSum;
uint16_t PM01ValueSum;
uint16_t PM2_5ValueSum;
uint16_t PM10ValueSum;

double CO2concSum;

float tempCsum;
float tempFsum;
float humiditySum;

// For chronodot rtc
RTC_DS3231 RTC;

// SD logging
const int chipSelect = 10;
File myFile;
String dataString = ""; // make a string for assembling the data to log:
boolean resetFirstTime;

String firstDataString = "";

//For COZIR
double val;
double CO2conc = 0.0;
double FS = 2000;
double vsupply = 1023;

//For PM (plantower)
uint16_t TPM01Value = 0;        //define PM1.0 variable
uint16_t TPM2_5Value = 0;       //define PM2.5 variable
uint16_t TPM10Value = 0;       //define PM10 variable
uint16_t PM01Value = 0;        //define PM1.0 variable
uint16_t PM2_5Value = 0;       //define PM2.5 variable
uint16_t PM10Value = 0;       //define PM10 variable
uint8_t receiveDat[24];       //define array to receive data from PMS3003
uint8_t testvalue;
int cnt = 0;
boolean pmFailed = false;
int pmErrorCount = 0;

//For temp/rh sensor (sht1x)
float tempC = 0;
float tempF = 0;
float humidity = 0;
int numBadReadings = 0;
const int dataPin = 4;
const int clockPin = 3;
SHT1x sht1x(dataPin, clockPin); //(Data, SCK)

//Alphasense Electrochemical gas sensors
float workNO, worksumNO, auxNO, auxsumNO, workCO, worksumCO, auxCO, auxsumCO, NO, CO, sumNO, sumCO, avgNO, avgCO = 0;
float workNO2, worksumNO2, auxNO2, auxsumNO2, workO3, worksumO3, auxO3, auxsumO3, NO2, O3, avgNO2, avgO3, sumNO2, sumO3 = 0;
float NOsum, COsum, NO2sum, O3sum, NOavg, COavg, NO2avg, O3avg;
//below pins for o3/no2
int workNO2pin = A0; // GND SN1
int auxNO2pin = A1; // 6V SN1 
int workO3pin = A2; // GND SN2 
int auxO3pin = A3; // 6V SN2 

//below for NO/CO
int workNOpin = A6;//A12; // GND SN1
int auxNOpin = A7;//A13; // 6V SN1
int workCOpin = A8;//A14; // GND SN2
int auxCOpin = A9;//A15; // 6V SN2

void setup() {
  Serial.begin(9600); // Initialize Baud rate
  Serial1.begin(9600);
  delay(3000); // delay just to have time to open serial monitor

  //Initialize LEDs
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  
  //----------SD SETUP-------------
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // keep flashing an led
    while (true) {
      digitalWrite(redPin, HIGH);
      delay(300);
      digitalWrite(redPin, LOW);
      delay(300);
    }
    // don't do anything more:
    return;
  }

  Serial.println("done.");


  //--------COZIR SETUP------------
  Serial.print("Initializing COZIR...");
  pinMode(A14, INPUT);
  Serial.println("done");

  //--------RTC SETUP ------------
  //Need to add RTC checks
  Serial.print("Initializing RTC Chronodot...");
  //LED lights on before RTC is set up
  digitalWrite(orangePin, HIGH);
  
  Wire.begin();
  RTC.begin();
  //if RTC gets initialized, turn LED off
  digitalWrite(orangePin, LOW);
  
//  Serial.print("Doing RTC checks...");
//Check if RTC is running
//  if (! RTC.isrunning()) {
//    Serial.println("RTC is NOT running!");
//    // following line sets the RTC to the date & time this sketch was compiled
//    //ONLY UNCOMMENT BELOW IF TIME NOT ALREADY SET!!!
//    //RTC.adjust(DateTime(__DATE__, __TIME__));
//  }
  Serial.print("Setting up RTC now...");
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time!  Updating");
    //ONLY UNCOMMENT BELOW IF TIME NOT ALREADY SET!!!
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  char datastr[100];
  RTC.getControlRegisterData( datastr[0]  );
  Serial.println("done");



  now = RTC.now();

  // SD logging string
  firstDataString = String(now.year(), DEC);
  firstDataString += "/";
  firstDataString += String(now.month(), DEC);
  firstDataString += "/";
  firstDataString += String(now.day(), DEC);
  firstDataString += " ";
  firstDataString += String(now.hour(), DEC);
  firstDataString += ":";
  firstDataString += String(now.minute(), DEC);
  firstDataString += ":";
  firstDataString += String(now.second(), DEC);
  firstDataString += " ";

  // Initial log each time teensy is plugged in.
  myFile = SD.open(buffer, FILE_WRITE);
  String firstStr = "Box with multiple sensors...New Logging Session..." + firstDataString;
  sdLog(buffer, firstStr);
}

/**** LOOP BEGINS HERE ****//**** LOOP BEGINS HERE ****//**** LOOP BEGINS HERE ****/


void loop() {
  if (resetFirstTime) {
    firstTime = millis();
    resetFirstTime = false;
  }
  /*
   * PMS3003 Plantower PM Sensor
   * Readings in PM1.0, 2.5, 10.0
   * Format: TSI_1 TSI_2.5 TSI_10 PM1 PM2.5 PM10
   */
  if (Serial1.available() > 0) {
    testvalue = Serial1.read();

    //if PM previously wasn't working but is now, turn off yellow LED
    digitalWrite(yellowPin, LOW);

    //Make sure pm data serial reads are aligned starting from 42 (in hex)

    cnt = cnt + 1; // increment count until 24 for pm readings
    receiveDat[cnt] = testvalue;   //receive 1 of 24 bytes from air detector module

    //troubleshoot this check
    //    if (receiveDat[16] == 66 && receiveDat[17] == 77) {
    //      receiveDat[16] = 0;
    //      receiveDat[17] = 0;
    //      cnt = 0;
    //    }

    //how does 2nd work if starts from 0
    //    if (receiveDat[1] != 66 && receiveDat[2] != 77) {
    //      cnt = 0;
    //    }

    //first 4 bytes are always 42, 4D, 0, 14...or in dec : 66,77,0,20

    //Check if first byte is 66. If not, start reading measurement over again
    if (receiveDat[1] != 66) {
      pmErrorCount++;
      cnt = 0;
    }

    if (cnt == 24)
    {
      Serial.println("*********************");
      /*
      *  Real-time Clock (DS3231) Chronodot --- Date & Time
      *  format: yyyy/mm/dd hh:mm:ss
      */
      DateTime now = RTC.now();
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(' ');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.print(" ");

      //PM Readings
      cnt = 0;
      TPM01Value = receiveDat[5] * 256 + receiveDat[6]; //calculate "TSI" PM1.0 concentration
      TPM2_5Value = receiveDat[7] * 256 + receiveDat[8]; //calculate "TSI" PM2.5 concentration
      TPM10Value = receiveDat[9] * 256 + receiveDat[10]; //calculate "TSI" PM10 concentration

      TPM01ValueSum += TPM01Value;
      TPM2_5ValueSum += TPM2_5Value;
      TPM10ValueSum += TPM10Value;

      //    Serial.print("{TSI 01 = ");
      Serial.print(TPM01Value);
      Serial.print(" ");
      //    Serial.print(", TSI 2.5 = ");
      Serial.print(TPM2_5Value);
      Serial.print(" ");
      //    Serial.print(", TSI 10 = ");
      Serial.print(TPM10Value);
      Serial.print(" ");
      PM01Value = receiveDat[11] * 256 + receiveDat[12]; //calculate PM1.0 concentration
      PM2_5Value = receiveDat[13] * 256 + receiveDat[14]; //calculate PM2.5 concentration
      PM10Value = receiveDat[15] * 256 + receiveDat[16]; //calculate PM10 concentration
      // ****************************
      //CONSIDER PUTTING IN IF CONDITION FOR ABSURDLY HIGH PM VALUES THAT MAY THROW OFF AVGS
      // ****************************
      PM01ValueSum += PM01Value;
      PM2_5ValueSum += PM2_5Value;
      PM10ValueSum += PM10Value;

      //    Serial.print(", PM01 Value = ");
      Serial.print(PM01Value);
      Serial.print(" ");
      //    Serial.print(", PM2.5 Value = ");
      Serial.print(PM2_5Value);
      Serial.print(" ");
      //    Serial.print(", PM10 Value = ");
      Serial.print(PM10Value);
      Serial.print(" ");
      //    Serial.println("}");

      /*
      * COZIR CO2 Sensor
      * Readings in ???
      */

      val = analogRead(A14);
      CO2conc = FS * (val / vsupply);
      CO2concSum += CO2conc;
      Serial.print("CO2=");
      Serial.print(CO2conc);
      Serial.print(" ");

      /*
       * SHT1X Temp/RH sensor
       * Readings in Fareinheit and % humidity
       */
      //      readhum();
      //              tempCsum = tempC + tempCsum;
      //      Serial.print("tempC=");
      //      Serial.print(tempC);
      //      Serial.print(" ");
      //      //              RHsum = RH + RHsum;
      //      Serial.print("RH=");
      //      Serial.print(RH);
      //      Serial.println("");


      //data logging for temp/rh
      //      dataString += tempC;
      //      dataString += " ";
      //      dataString += RH;
      //      dataString += " ";



      /*  Below is alternative code using sht1x library features
       *  However, this library has delays that throw off loop timing
       */

      tempC = sht1x.readTemperatureC();
      tempCsum += tempC;
      tempF = sht1x.readTemperatureF();
      tempFsum += tempF;
      humidity = sht1x.readHumidity();
      //      if (humidity < 0 || humidity > 100) {
      //        humidity = 0;
      //        numBadReadings++;
      //      }
      humiditySum += humidity;
      Serial.print(" Temp = ");
      //      Serial.print(tempF);
      //      Serial.print(" ");
      //      Serial.print("F, ");
      Serial.print(tempC);
      Serial.print(" ");
      Serial.print("C");
      Serial.print(" Humidity = ");
      Serial.print(humidity);
      Serial.print(" ");
      Serial.print("%");

      //Electrochem
      workNO = analogRead(workNOpin) * 4.9;
      auxNO = analogRead(auxNOpin) * 4.9;
      workCO = analogRead(workCOpin) * 4.9;
      auxCO = analogRead(auxCOpin) * 4.9;

      workNO2 = analogRead(workNO2pin) * 4.9;
      auxNO2 = analogRead(auxNO2pin) * 4.9;
      workO3 = analogRead(workO3pin) * 4.9;
      auxO3 = analogRead(auxO3pin) * 4.9;

      NO = ((workNO - 271) - (auxNO - 270)) / 0.352;
      CO = ((workCO - 280) - (auxCO - 279)) / 0.243;
      NO2 = ((workNO2 - 298) - (auxNO2 - 294)) / 0.456;
      O3 = (((workO3 - 412) - (auxO3 - 407)) - NO2 * 0.333) / 0.222;

      Serial.print(" NO = ");
      Serial.print(NO);
      Serial.print(" CO = ");
      Serial.print(CO);
      Serial.print(" NO2 = ");
      Serial.print(NO2);
      Serial.print(" O3 = ");
      Serial.println(O3);

      NOsum += NO;
      COsum += CO;
      NO2sum += NO2;
      O3sum += O3;

      sampleSize++;

      //Debugging purposes
      for(int x = 1; x <= 24; x++){
        Serial.print(receiveDat[x]);
        Serial.print(" ");
      }
      Serial.println();


      logTime = millis();
      timeElapsed = logTime - firstTime;
      Serial.print("Time elapsed: ");
      Serial.println(timeElapsed);
      long intervalTime = timeElapsed - prevTimeElapsed;
      Serial.print("Time from last measurement: ");
      Serial.println(intervalTime);
      prevTimeElapsed = timeElapsed;
      Serial.print("Number of errors with PM: ");
      Serial.println(pmErrorCount);
      pmErrorCount = 0;
//      Serial.print("Sample Size = ");
//      Serial.println(sampleSize);


      //if time elapsed is greater than a minute, record averages
      //Serial.print("Checking if greater than a minute has elapsed...");

      if (timeElapsed > 58500) {
        //record averages
        Serial.print("A minute has elapsed...");
        TPM01ValueAvg = TPM01ValueSum / sampleSize;
        TPM2_5ValueAvg = TPM2_5ValueSum / sampleSize;
        TPM10ValueAvg = TPM10ValueSum / sampleSize;
        PM01ValueAvg = PM01ValueSum / sampleSize;
        PM2_5ValueAvg = PM2_5ValueSum / sampleSize;
        PM10ValueAvg = PM10ValueSum / sampleSize;

        CO2concAvg = CO2concSum / sampleSize;

        tempCavg = tempCsum / sampleSize;
        tempFavg = tempFsum / sampleSize;
        humidityAvg = humiditySum / (sampleSize - numBadReadings);

        NOavg = NOsum / sampleSize;
        COavg = COsum / sampleSize;
        NO2avg = NO2sum / sampleSize;
        O3avg = O3sum / sampleSize;

        Serial.println("Calculated avgs...");


        dataString = String(now.year(), DEC);
        dataString += "/";
        dataString += String(now.month(), DEC);
        dataString += "/";
        dataString += String(now.day(), DEC);
        dataString += ", ";
        dataString += String(now.hour(), DEC);
        dataString += ":";
        dataString += String(now.minute(), DEC);
        dataString += ":";
        dataString += String(now.second(), DEC);
        dataString += ", ";
        if (pmFailed) {
          for (int repeatCount = 0; repeatCount < 6; repeatCount++) {
            dataString += "NA";
            dataString += ", ";
          }
          pmFailed = false;
        }
        else {
          dataString += String(TPM01ValueAvg) + ", " + String(TPM2_5ValueAvg) + ", " + String(TPM10ValueAvg);
          dataString += ", " + String(PM01ValueAvg) + ", " + String(PM2_5ValueAvg) + ", " + String(PM10ValueAvg);
        }

        dataString += ", " + String(CO2concAvg, 2) + ", " + String(tempCavg, 2) + ", " + String(humidityAvg, 2);
        dataString += ", " + String(NOavg) + ", " + String(COavg) + ", " + String(NO2avg) + ", " + String(O3avg);

        Serial.println("----------------------------------------");
        Serial.println("--------MINUTE AVERAGE below------------");
        Serial.println(dataString);
        Serial.println("----------------------------------------");
        Serial.println("----------------------------------------");

        sdLog(buffer, dataString);


        //reset first time
        resetFirstTime = true;

        //reset sum values
        TPM01ValueSum = 0;
        TPM2_5ValueSum = 0;
        TPM10ValueSum = 0;
        PM01ValueSum = 0;
        PM2_5ValueSum = 0;
        PM10ValueSum = 0;

        CO2concSum = 0;

        tempCsum = 0;
        tempFsum = 0;
        humiditySum = 0;

        NOsum = 0;
        NO2sum = 0;
        O3sum = 0;
        COsum = 0;

        //reset average values
        TPM01ValueAvg = 0;
        TPM2_5ValueAvg = 0;
        TPM10ValueAvg = 0;
        PM01ValueAvg = 0;
        PM2_5ValueAvg = 0;
        PM10ValueAvg = 0;

        CO2concAvg = 0;

        tempCavg = 0;
        tempFavg = 0;
        humidityAvg = 0;

        sampleSize = 0;
        numBadReadings = 0;
      }

      dataString = ""; //empty dataString for next set of measurements
      
      Serial.println();
      Serial.println("*********************");
    }
  }

  /*
   *  IF SERIAL FAILS FOR PLANTOWER, DO ALL OTHER MEASUREMENTS BUT PRINT NA FOR PLANTOWER
   * READINGS.
   */
  else {
    /*
      *  Real-time Clock (DS3231) Chronodot --- Date & Time
      *  format: yyyy/mm/dd hh:mm:ss
      */

    pmFailed = true;

    //Yellow LED on indicating PM not working
    digitalWrite(yellowPin, HIGH);

    DateTime now = RTC.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(" ");

    //    Serial.print("{TSI 01 = ");
    Serial.print("NA");
    Serial.print(" ");
    //    Serial.print(", TSI 2.5 = ");
    Serial.print("NA");
    Serial.print(" ");
    //    Serial.print(", TSI 10 = ");
    Serial.print("NA");
    Serial.print(" ");

    //    Serial.print(", PM01 Value = ");
    Serial.print("NA");
    Serial.print(" ");
    //    Serial.print(", PM2.5 Value = ");
    Serial.print("NA");
    Serial.print(" ");
    //    Serial.print(", PM10 Value = ");
    Serial.print("NA");
    Serial.print(" ");
    //    Serial.println("}");

    /*
    * COZIR CO2 Sensor
    * Readings in ???
    */

    val = analogRead(A14);
    CO2conc = FS * (val / vsupply);
    CO2concSum += CO2conc;
    Serial.print("CO2=");
    Serial.print(CO2conc);
    Serial.print(" ");

    /*  Below is alternative code using sht1x library features
     *  However, this library has delays that throw off loop timing
     */

    tempC = sht1x.readTemperatureC();
    tempCsum += tempC;
    tempF = sht1x.readTemperatureF();
    tempFsum += tempF;
    humidity = sht1x.readHumidity();

    //Below checks for unfeasible humidity values
    //      if (humidity < 0 || humidity > 100) {
    //        humidity = 0;
    //        numBadReadings++;
    //      }

    humiditySum += humidity;
    Serial.print(" Temp = ");
    //      Serial.print(tempF);
    //      Serial.print(" ");
    //      Serial.print("F, ");
    Serial.print(tempC);
    Serial.print(" ");
    Serial.print("C");
    Serial.print(" Humidity = ");
    Serial.print(humidity);
    Serial.print(" ");
    Serial.print("%");

    //Electrochem
    workNO = analogRead(workNOpin) * 4.9;
    auxNO = analogRead(auxNOpin) * 4.9;
    workCO = analogRead(workCOpin) * 4.9;
    auxCO = analogRead(auxCOpin) * 4.9;

    workNO2 = analogRead(workNO2pin) * 4.9;
    auxNO2 = analogRead(auxNO2pin) * 4.9;
    workO3 = analogRead(workO3pin) * 4.9;
    auxO3 = analogRead(auxO3pin) * 4.9;

    NO = ((workNO - 271) - (auxNO - 270)) / 0.352;
    CO = ((workCO - 280) - (auxCO - 279)) / 0.243;
    NO2 = ((workNO2 - 298) - (auxNO2 - 294)) / 0.456;
    O3 = (((workO3 - 412) - (auxO3 - 407)) - NO2 * 0.333) / 0.222;

    Serial.print(" NO = ");
    Serial.print(NO);
    Serial.print(" CO = ");
    Serial.print(CO);
    Serial.print(" NO2 = ");
    Serial.print(NO2);
    Serial.print(" O3 = ");
    Serial.println(O3);

    NOsum += NO;
    COsum += CO;
    NO2sum += NO2;
    O3sum += O3;

    sampleSize++;

    logTime = millis();
    timeElapsed = logTime - firstTime;
    Serial.print("Time elapsed: ");
    Serial.println(timeElapsed);
    Serial.print("Sample Size = ");
    Serial.println(sampleSize);


    //if time elapsed is greater than a minute, record averages
    Serial.print("Checking if greater than a minute has elapsed...");

    if (timeElapsed > 58500) {
      //record averages
      Serial.print("A minute has elapsed...");

      CO2concAvg = CO2concSum / sampleSize;

      tempCavg = tempCsum / sampleSize;
      tempFavg = tempFsum / sampleSize;
      humidityAvg = humiditySum / (sampleSize - numBadReadings);

      NOavg = NOsum / sampleSize;
      COavg = COsum / sampleSize;
      NO2avg = NO2sum / sampleSize;
      O3avg = O3sum / sampleSize;

      Serial.println("Calculated avgs...");


      dataString = String(now.year(), DEC);
      dataString += "/";
      dataString += String(now.month(), DEC);
      dataString += "/";
      dataString += String(now.day(), DEC);
      dataString += ", ";
      dataString += String(now.hour(), DEC);
      dataString += ":";
      dataString += String(now.minute(), DEC);
      dataString += ":";
      dataString += String(now.second(), DEC);
      dataString += ", ";
      for (int repeatCount = 0; repeatCount < 6; repeatCount++) {
        dataString += "NA";
        dataString += ", ";
      }
      dataString += ", " + String(CO2concAvg, 2) + ", " + String(tempCavg, 2) + ", " + String(humidityAvg, 2);
      dataString += ", " + String(NOavg) + ", " + String(COavg) + ", " + String(NO2avg) + ", " + String(O3avg);

      Serial.println("----------------------------------------");
      Serial.println("--------MINUTE AVERAGE below------------");
      Serial.println(dataString);
      Serial.println("----------------------------------------");
      Serial.println("----------------------------------------");

      sdLog(buffer, dataString);

      //reset first time
      resetFirstTime = true;

      //reset sum values

      CO2concSum = 0;

      tempCsum = 0;
      tempFsum = 0;
      humiditySum = 0;

      NOsum = 0;
      NO2sum = 0;
      O3sum = 0;
      COsum = 0;

      //reset average values
      CO2concAvg = 0;

      tempCavg = 0;
      tempFavg = 0;
      humidityAvg = 0;

      sampleSize = 0;
      numBadReadings = 0;
    }
    dataString = ""; //empty dataString for next set of measurements
    Serial.println();
  }


}

//should create a method for logging

void sdLog(const char* fileName, String stringToWrite) {
  File myFile = SD.open(fileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to ");
    Serial.print(fileName);
    Serial.print("...");
    myFile.println(stringToWrite);
    // close the file:
    myFile.close();
    Serial.println("done.");
    digitalWrite(redPin, LOW);
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(fileName);
    //turn on redLED
    //this means that the SD card was in/powered when the program started
    //but it got disconnected somehow
    digitalWrite(redPin, HIGH);
  }
}


