//BLYNK İÇİN GEREKLİ KÜTÜPHANELER
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//BLYNK ve NODEMCU ESP8266 BAĞLANTISI İÇİN GEREKLİ BİLGİLER
char auth[] = "";        // kendi token adresinizi girin
char ssid[] = "";        // Wifi ağının adı
char pass[] = "";        // Wifi ağının şifresi

//DHT11 SICAKLIK SENSÖRÜ İÇİN KÜTÜPHANE
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

#define fanMotorPin 16 // DO = 16 Fan motorunun sinyal bacağının bağlandığı pin 


//toprak nem için belirli bir nem değeri belirledik = 600
//nem değeri 600 ün üzerine çıktığı zaman motor çalışıp 3 sn su basıp  5 sn bekleyecek ve yeterli gelmediğinde tekrar su basacak
#define maviLedPin 14 // D5 = 14 kuru iken yanar ve su pompası çalışmalı
#define beyazLedPin 15 // D8 = 15 nemli iken yanar
#define suMotorPin 13 // D7 = 13
#define toprakSensorPin 2 // D4 = 2

int toprakNemveri;

//UBIDOTS BULUT PLATFORMU İÇİN GEREKLİ KÜTÜPHANE
#include "UbidotsMicroESP8266.h"

////UBIDOTS BULUT PLATFORMU ve NODEMCU ESP8266 BAĞLANTISI İÇİN GEREKLİ BİLGİLER
#define TOKEN  ""   // Ubidots TOKEN adresinizi girin
#define WIFISSID "" // Wifi ağının adı
#define PASSWORD "" // Wifi ağının şifresi


Ubidots client(TOKEN);

unsigned long lastMillis = 0;

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

//blynk'te sanal olarak pinlerimizi tanımladık DHT11 sensörü için
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t); 
  
 //***************************************/ 
//***************************************/
// - ORTAM SICAKLIĞINA GÖRE FAN ÇALIŞMA SİSTEMİ
 
  if ( t > 30)                       
            {
              digitalWrite (fanMotorPin , HIGH);
              Serial.println("Fan devreye girdi! Bildirim gönderiliyor");
              Blynk.notify("Fan sistemi devreye girdi! Sıcaklık kontrol altında :D"); //BLYNK mobil uygulama üzerinden kullanıcıya bildirim mesajı gönderiliyor     
           }
            else 
            {
               digitalWrite (fanMotorPin , LOW);          
            }

//***************************************/       
//***************************************/
//TOPRAK NEM MİKTARINA GÖRE SU POMPASI ÇALIŞMA SİSTEMİ

if(toprakNemveri>600)
{
  Serial.println("Topraginizi sulayin!, bildirim gonderiliyor");
  Blynk.notify("Topraginizi sulayin!");
  delay(3000);
  digitalWrite(suMotorPin,HIGH);
  digitalWrite(maviLedPin, HIGH);
    digitalWrite(beyazLedPin, LOW);
  delay(3000);
  digitalWrite(suMotorPin,LOW);
  delay(3000);
  toprakNemveri=analogRead(toprakSensorPin);
  if(toprakNemveri>600){
    Serial.println("Toprak halen sulanmadı :(");
    Blynk.notify("Toprak halen sulanmadı :(");
      delay(3000);
    digitalWrite(suMotorPin,HIGH);
    delay(2000);
    digitalWrite(suMotorPin,LOW);
    delay(5000);
    }else {
      digitalWrite(beyazLedPin, HIGH);
      digitalWrite(suMotorPin,LOW);
      digitalWrite(maviLedPin, LOW);
    Serial.println("Sulama gerceklestirildi! Topraginiz nemli :D");
    Blynk.notify("Sulama gerceklestirildi! Topraginiz nemli :D");
      delay(3000);
      }
  }else{
    digitalWrite(beyazLedPin, HIGH);
    digitalWrite(suMotorPin,LOW);
    digitalWrite(maviLedPin, LOW);
    Serial.println("Sulama gerceklestirildi! Topraginiz nemli :D");
    Blynk.notify("Sulama gerceklestirildi! Topraginiz nemli :D");
      delay(3000);
    }
    }
//****************************************/
//***************************************/      

void setup(){
    Serial.begin(115200);
      pinMode (fanMotorPin, OUTPUT);
      pinMode(maviLedPin , OUTPUT);
      pinMode(beyazLedPin ,OUTPUT);
      pinMode(suMotorPin , OUTPUT);
       
       Blynk.begin(auth, ssid, pass);
       dht.begin();
       timer.setInterval(1000L, sendSensor);
       delay(10);
       client.wifiConnection(WIFISSID, PASSWORD);
}
void loop(){
  Blynk.run();
  timer.run();
  
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
  toprakNemveri=analogRead(toprakSensorPin);
  Serial.print("toprakNemveri degeri: ");
  Serial.println(toprakNemveri);
  
    //ubidots için anlık DHT11 değerleri alınıyor
    if (millis() - lastMillis > 10000) //every 10S
    { 
            lastMillis = millis();
            client.add("nem",h );
            client.add("sicaklik",t );
            client.add("toprakNem", toprakSensorPin);
            client.sendAll(true);
    }
           
   Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t); 
  Blynk.virtualWrite(V7, toprakNemveri); 
}
