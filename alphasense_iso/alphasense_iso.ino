//Alphasense Electrochemical gas sensors
float workNO, worksumNO, auxNO, auxsumNO, workCO, worksumCO, auxCO, auxsumCO, NO, CO, sumNO, sumCO, avgNO, avgCO = 0;
float workNO2, worksumNO2, auxNO2, auxsumNO2, workO3, worksumO3, auxO3, auxsumO3, NO2, O3, avgNO2, avgO3, sumNO2, sumO3 = 0;
int workNO2pin = A7; // GND SN2 CO2
int auxNO2pin = A6; // 6V SN1 CO2
int workO3pin = A1; // GND SN2 O3
int auxO3pin = A0; // 6V SN2 O3

int workNOpin = A9;//A12; // GND SN1 NO
int auxNOpin = A8;//A13; // 6V SN1 NO
int workCOpin = A3;//A14; // GND SN2 CO
int auxCOpin = A2;//A15; // 6V SN2 CO

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
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

  Serial.print("NO = ");
  Serial.println(NO);
  Serial.print("CO = ");
  Serial.println(CO);
  Serial.print("NO2 = ");
  Serial.println(NO2);
    Serial.print("O3 = ");
  Serial.println(O3);
  delay(1000);
}
