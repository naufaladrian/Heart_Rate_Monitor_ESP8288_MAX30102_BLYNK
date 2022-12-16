
//library wifi
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_AUTH_TOKEN "w4DYxz_Slg8wt8txC_3VHPPyTadzC4SH" //blynk auth token

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "namawifi";//WIFI name
char pass[] = "passwordwifi";//WIFI password

//library max30102
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

//variabel untuk menghitung detak jantung
const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; //Array heart rates
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg;

void setup()
{
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);//konfigurasi blynk

  Serial.begin(115200);
  Serial.println("Initializing...");

  //kondisi jika sensor tidak bekerja
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Konfigurasi sensor
  particleSensor.setPulseAmplitudeRed(0x0A); //menyalakan led warna merrah jika sensor berjalan
  particleSensor.setPulseAmplitudeGreen(0); 
}

void loop()
{
  Blynk.run();//menjalankan blynk
  long irValue = particleSensor.getIR();//mendapatkan nilai infra red dari sensor

  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    //menghitung BPM
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; 
      rateSpot %= RATE_SIZE; 

      //Menghitung Rata2 
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  //memunculkan output ke blynk  
  Blynk.virtualWrite(V4, beatsPerMinute);//output bpm datastream v4
  Blynk.virtualWrite(V5, beatAvg);//output rata2 bpm datastream v5

  //memunculkan output ke serial monitor  
  Serial.print("IR=");
  Serial.print(irValue);//memunculkan nilai infra red di serial monitor
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);//memunculkan nilai BPM di serial monitor
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);//memunculkan rata2 BPM di serial monitor

  //kondisi jika sensor tidak mendeteksi jari
  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
}


