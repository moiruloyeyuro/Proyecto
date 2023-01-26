#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h> // librerias pantalla
#include "Arduino.h"
#include <MPU9250_asukiaaa.h>
MPU9250_asukiaaa mySensor; //librerias sensor
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h> 
#include <ESP32_FTPClient.h>
#include <StreamUtils.h>
#include "FS.h"
#include "SD.h"
#include <Math.h>

//socket
const uint16_t puerto=23;
WiFiClient client;
String opcion;


#define sclk 14
#define mosi 13
#define cs   15
#define rst  4
#define dc   16 
#define GREEN           0x07E0
#define BLACK           0x0000
#define WHITE           0xFFFF //Declaraciones pantalla

const char* ssid = "Redmi 8";
const char* password = "patorulo"; //Declaraciones wifi


char ftp_server[] = "192.168.43.86";
char ftp_user[]   = "esp32";
char ftp_pass[]   = "yanuro11";


// Definición de uso de los pines
const int pinSDA= 22;
const int pinSCL= 23;
double AcX, AcY, AcZ;
boolean grabar = false;
int cont;
int result;
int muestra = 30;


char nombrefin[20];


const int pinBoton = 36;
boolean estadoBoton = 1;

Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);



void setup(){ 

  
  Serial.begin(115200);   
  display.begin(); //prepara pantalla
  
  pinMode(pinBoton, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Wire.begin(pinSDA, pinSCL);
  mySensor.setWire(&Wire);
  mySensor.beginAccel(); 
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
//
  Serial.print("Conectando a Hercules"); //conecto a hercules
  while (!client.connect(ftp_server, puerto)) {
        Serial.print(".");
        delay(1000);
    }
  client.println("Aqui ESP32");

}


 
void loop() {    
   
  if (grabar == false) { 
  estadoBoton = digitalRead(pinBoton);     
    if (estadoBoton == 0) {
         Serial.println("Grabo muestra");
         grabar = true;
         xTaskCreate(Muestreo,"Muestreo",10000,NULL,1,NULL);
          }
         }
         
    delay(100);   
    } 





    
//Funcion muestreo
void Muestreo( void * parameter )
{
  float x[200];
  float y[200];
  float z[200];
  int tiempo = 0;
  int i=0;
  int av=0;
  int lat=0;
  int alt=0;
  String prediccion;
  delay(1000);
  client.println("voy a grabar");
  display.fillScreen(GREEN);
  while(grabar){
        result = mySensor.accelUpdate();
        x[i] = mySensor.accelX()*1000;
        if ((x[i]>5000) or (x[i]<-5000)){
          av=av+1;
        }
        y[i] = mySensor.accelY()*1000;
        if ((y[i]>5000) or (y[i]<-5000)){
          lat = lat+1;
        }
        z[i] = mySensor.accelZ()*1000;
        if ((z[i]>5000) or (z[i]<-5000)){
          alt=alt+1;
        }
        tiempo = tiempo + 10;
        i=i+1;
        
        delay(10); //muestreo cada 10ms
        estadoBoton = digitalRead(pinBoton); 
        if ((estadoBoton == 0)or(tiempo == 2000)){ 
          client.println("Dejo de grabar");
          display.fillScreen(BLACK);
          display.setTextColor(WHITE);
          display.setCursor(0, 5);
          display.setTextSize(2);
          grabar = false;
          if (alt<1){
              display.println("Bote"); 
             } 
            else if (lat>5){
              if (alt+lat+av<27){
               display.println("Pivote");
            }
            else{
               display.println("Salto");
          }
          }
           if ((alt+lat+av<26) and (lat<6)){
              display.println("Penal"); 
            }
            else if (lat<5){
              display.println("Cadera");
          }
          
          else {
             display.println("No se");
          }
          


          delay(3000);  
        
      }
   
  }
  vTaskDelete( NULL );
  }
