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
#define WHITE           0xFFFF
#define  BLACK           0x0000 //Declaraciones pantalla

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
int muestra = 0;


char nombrefin[20];


#define  GREEN           0x07E0
#define  WHITE           0xFFFF

const int pinBoton = 36;
boolean estadoBoton = 1;




    
Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);
ESP32_FTPClient ftp (ftp_server,ftp_user,ftp_pass, 5000, 2);



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
    ftp.OpenConnection();

}

 
void loop() {    
   
  if (grabar == false) { 
  estadoBoton = digitalRead(pinBoton);     
    if (estadoBoton == 0) {
         
         muestra=muestra+1;
         Serial.println("Grabo muestra " + String(muestra));
         grabar = true;
         xTaskCreate(Muestreo,"Muestreo",10000,NULL,1,NULL);
          }
         }
         
    delay(100);   
    } 





    
//Funcion muestreo
void Muestreo( void * parameter )
{
  DynamicJsonDocument doc(20000); 
  JsonObject Sensor = doc.createNestedObject("Acelerometro");
  JsonArray x = Sensor.createNestedArray("x");
  JsonArray y = Sensor.createNestedArray("y");
  JsonArray z = Sensor.createNestedArray("z");
  int tiempo = 0;
  delay(1000);
  display.fillScreen(GREEN);
  while(grabar){
        result = mySensor.accelUpdate();
        AcX = mySensor.accelX()*1000;
        x.add(int(AcX));
        AcY = mySensor.accelY()*1000;
        y.add(int(AcY));
        AcZ = mySensor.accelZ()*1000;
        z.add(int(AcZ));
        tiempo = tiempo + 10;
        
        
        delay(10); //muestreo cada 10ms
        estadoBoton = digitalRead(pinBoton); 
        if ((estadoBoton == 0)or(tiempo == 2000)){ 

     
          client.println("Dejo de grabar");
          display.fillScreen(BLACK);
          
          //xTaskCreate(json,"json",10000,NULL,1,NULL);
          char escrito[8000];
          String nombre[25];
          String nombredoc = "muestra_";
          nombredoc = nombredoc + String(muestra) + ".json";
          serializeJsonPretty(doc, escrito);
          nombredoc.toCharArray(nombrefin,20);
          
          ftp.InitFile("Type A");
          ftp.NewFile(nombrefin); 
          ftp.Write(escrito);
          ftp.CloseFile();
          client.println("escrito");
          grabar = false;
          delay(3000);  
        }
      }
   vTaskDelete( NULL );
  }
  
