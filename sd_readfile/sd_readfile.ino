/*
  SD card file dump

 This example shows how to read a file from the SD card using the
 SD library and send it over the serial port.

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created  22 December 2010
 by Limor Fried
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>

const char* buffer = "text.txt";
const int chipSelect = 10;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  Serial.println("-----------------------");
  File dataFile = SD.open(buffer, FILE_READ);
  // if the file is available, write to it:
  Serial.print("dataFile = ");
  Serial.println(dataFile);
  if (dataFile) {
    Serial.print("dataFile.available() = ");
    Serial.println(dataFile.available());
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print("error opening ");
    Serial.println(buffer);
    //Create file if doesn't exist
    Serial.print("Error, so creating file...");
    dataFile = SD.open(buffer, FILE_WRITE);
    if (dataFile){
      Serial.println("done.");
    }
    else{
      Serial.println("failed to create file.");
    }
  }
  Serial.println("-----------------------");
}

void loop() {
}
