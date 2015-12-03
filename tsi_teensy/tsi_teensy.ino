//AVERAGE EVERY MINUTE
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include <SD.h>
#include <SD_t3.h>

RTC_DS3231 RTC;

String year, month, day, second, hour, minute;
float measurement, pm, pmtotal, pmAverage, voltage, current, total;
int j;
int numLoops;
File myFile;

String writeString;
const char* buffer = "test.txt";

long currentTime, previousTime;

void setup() {
  //set the serial's Baudrate
  Serial.begin(9600);
  delay(1000);

  //Initialize counter and pm total variables
  j = 1;
  pmtotal = 0;

  //--------RTC SETUP ------------
  Serial.print("Initializing RTC Chronodot...");
//  delay(1000);
  Wire.begin();
  RTC.begin();
  Serial.print("Doing RTC checks...");
//  delay(1000);
  //Check if RTC is running
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
//    delay(1000);
    // following line sets the RTC to the date & time this sketch was compiled
    //ONLY UNCOMMENT BELOW IF TIME NOT ALREADY SET!!!
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  Serial.print("Setting up RTC now...");
//  delay(1000);
  DateTime now = RTC.now();
  previousTime = 0;
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time!  Updating");
//    delay(1000);
    //ONLY UNCOMMENT BELOW IF TIME NOT ALREADY SET!!!
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  char datastr[100];
  RTC.getControlRegisterData( datastr[0]  );
  Serial.println("done");
//  delay(1000);


  //---------SD Setup-------------------
  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

year = String(now.year(), DEC);
    month = String(now.month(), DEC);
    day = String(now.day(), DEC);
    hour = String(now.hour(), DEC);
    minute = String(now.minute(), DEC);
    second = String(now.second(), DEC);
    String logHeader = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second;
    sdLog(buffer, "TSI Box 1: New Logging Session - "+logHeader);
}



void loop()
{

  measurement = analogRead(A0);
  //  Serial.print("Analog Read 0 is: ");
  //  Serial.println(measurement);
  voltage = (measurement / 1023) * 3.32;
  //  Serial.print("Voltage = ");
  //  Serial.println(voltage);
  current = (voltage / 163.5) * 1000; // I = V/R (mult 1000 for mA)
  pmtotal += (300 - 5) / (20 - 4) * (current - 4);

  //    Serial.print("TSI Sensor PM level is: ");
  //    Serial.println(pm);
  DateTime now = RTC.now();
  
  if (now.second() != previousTime) { //If a second has passed
    
    previousTime = now.second(); // set previous time to current time
    year = String(now.year(), DEC);
    month = String(now.month(), DEC);
    day = String(now.day(), DEC);
    hour = String(now.hour(), DEC);
    minute = String(now.minute(), DEC);
    second = String(now.second(), DEC);
    
    Serial.print(year);
    Serial.print('/');
    Serial.print(month);
    Serial.print('/');
    Serial.print(day);
    Serial.print(' ');
    Serial.print(hour);
    Serial.print(':');
    Serial.print(minute);
    Serial.print(':');
    Serial.print(second);
    Serial.print(" ");

    pmAverage = pmtotal / j;

    //make sure pmAverage is positive in case of negative readings
    //    if (pmAverage < 0) {
    //      pmAverage = 0;
    //    }

    Serial.println(pmAverage);

    writeString = year + "/" + month + "/" + day + " " + hour + ":" + minute + ":" + second + " " + pmAverage;

    sdLog(buffer, writeString);

    j = 1;
    pmtotal = 0;
  }

  j++;
  delay(50);
}

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
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(fileName);
  }
}

