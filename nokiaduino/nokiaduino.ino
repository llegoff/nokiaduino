#include <dht.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <RTClib.h>
#include <Wire.h>
#include <SD.h>


#define DHT22_PIN 3
#define SW_PIN 5
 Adafruit_PCD8544 display = Adafruit_PCD8544(8, 10, 9);
 RTC_Millis RTC;
 dht DHT;
 unsigned long time;
 unsigned long aff_time;
 unsigned long SD_time;
 unsigned long SW_time;
 boolean sw_compteur = LOW;
 boolean prev_sw_compteur = LOW;
 boolean SD_OK = LOW;
 int compteur = 0;
 int temperature = 0;
 int humidite = 0;
 int correction = 0;
 float alim = 3.3;

static const unsigned char PROGMEM symbSDOK[] =
{ B11111100, 
  B10000010,
  B10000001, 
  B10000001,
  B10000001,
  B10000001,
  B11111111,
};

static const unsigned char PROGMEM symbSDKO[] =
{ B11111100, 
  B11111110,
  B11111111, 
  B11111111,
  B11111111,
  B11111111,
  B11111111,
};

static const unsigned char PROGMEM symbT[] =
{ B00111000, 
  B01000100,
  B01000100, 
  B01000100,
  B00111000,
  B00000000,
  B00111100,
  B01000010,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B01000010,
  B00111100 };

static const unsigned char PROGMEM symbH[] =
{ B01100010, 
  B10010100,
  B01101000, 
  B00010110,
  B00101001,
  B01000110,
  B00000000,
  B00010000,
  B00010000,
  B00010000,
  B00101000,
  B01000100,
  B01000100,
  B00111000};

 
void setup() {
  // put your setup code here, to run once:
  pinMode(SW_PIN,INPUT_PULLUP);
  display.begin();
  display.setContrast(55);
  display.display(); // show splashscreen
  display.clearDisplay();   // clears the screen and buffer
  RTC.begin(DateTime(__DATE__, __TIME__));
  SD_OK = SD.begin(6);
  delay(2000);
  aff_time=millis();
  SD_time=aff_time;
  SW_time=SD_time;
}

void loop() {
  time=millis();
  if (time > aff_time + 1000) {
    display.clearDisplay();
    niveauPile();
    Temp();
    dateEtHeure();
//    disp_compteur();
    display.display();
    aff_time = aff_time + 1000;
  }
  if (time > SD_time + 90000) {
    datalogger();
    SD_time=SD_time + 90000;
  }
  comptage();
}

void dateEtHeure() {
  DateTime now =RTC.now();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.print(now.day());
  display.print('/');
  display.print(now.month());
  display.print('/');
  display.print(now.year());
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(6,8);
  display.print(now.hour());
  display.print(':');
  display.print(now.minute());
  display.print(':');
  display.print(now.second());
}

void datalogger() {
  DateTime now =RTC.now();
  SD_OK = SD.begin(6);
  String chaine = "T";
  int i=now.year()-2000;
  if (i<10) chaine = chaine + "0";
  chaine = chaine + String(i);
  i=now.month();
  if (i<10) chaine = chaine + "0";
  chaine = chaine + String(i);
  i=now.day();
  if (i<10) chaine = chaine + "0";
  chaine = chaine + String(i);
  chaine = chaine + ".csv";
  if (SD_OK) {
    i=now.hour();
    chaine = String(i);
    i=now.minute();
    if (i<10) chaine = chaine + "0";
    chaine = chaine + String(i);
    chaine = chaine + ";";
    chaine = chaine + String(temperature);
    chaine = chaine + ";";
    chaine = chaine + String(humidite);
    File fichier = SD.open(chaine , FILE_WRITE);
    if (fichier.size()=0) fichier.println("Heure;Temperature;Humidite");
    else fichier.seek(fichier.size());
    fichier.println(chaine);
    fichier.close;
    }
  }
}

void comptage() {
  sw_compteur=!digitalRead(SW_PIN);
  if (sw_compteur != prev_sw_compteur) {
    if (millis()>(SW_time + 40)){
      if (sw_compteur==HIGH) compteur++;
      prev_sw_compteur=sw_compteur;
      SW_time=millis();
    }
  } 
}

void disp_compteur() {
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(10,16);
  display.print(compteur);
}


void niveauPile(){
  float bat= (float(analogRead(A7))*alim)/1024;
  display.drawRect(65,0,18,7,BLACK);
  display.drawRect(83,2,1,3,BLACK);
  if (bat>2) display.drawRect(67,2,2,3,BLACK);
  if (bat>2.2) display.drawRect(70,2,2,3,BLACK);
  if (bat>2.4) display.drawRect(73,2,2,3,BLACK);
  if (bat>2.6) display.drawRect(76,2,2,3,BLACK);
  if (bat>2.8) display.drawRect(79,2,2,3,BLACK);
}

void Temp(){
  int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
                display.setTextSize(2);
                display.setTextColor(BLACK);
                display.setCursor(4,32);
                temperature = DHT.temperature - correction;
		display.print((temperature),0);
                display.drawBitmap(28,32, symbT, 8, 14, BLACK);
                display.setCursor(46,32);
                humidite = DHT.humidity;
		display.print(humidite,0);
                display.drawBitmap(70,32, symbH, 8, 14, BLACK);
		break;
    case DHTLIB_ERROR_CHECKSUM: 
                display.setTextSize(1);
                display.setTextColor(BLACK);
                display.setCursor(0,32);
		display.print("Checksum error"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
                display.setTextSize(1);
                display.setTextColor(BLACK);
                display.setCursor(0,32);
		display.print("Time out error"); 
		break;
    default: 
                display.setTextSize(1);
                display.setTextColor(BLACK);
                display.setCursor(0,32);
		display.print("Unknown error"); 
		break;
  }
}
