#include <SoftwareSerial.h>
#define luz 14 //GPIO
#define presion 5 //GPIO
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "DIGIFIBRA-c54Q"
#define WIFI_PASSWORD "94GuTYXQeT"
// Insert Firebase project API Key
#define API_KEY "AIzaSyAaua17IgfeQY6Ji7svpyCofhWc-oecwqI"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://basededatos-f1968-default-rtdb.europe-west1.firebasedatabase.app/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


//Week Days
int weekDays[7]={6, 0, 1, 2, 3, 4, 5};

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;



//variables
int tiempocruzar = 3000;
int tiempoespera = 100;
int veces = 0;
int saltar = 0;
int comprobar = -1;
bool encendido = true;
unsigned long millisPrev = 0;

String fecha;

String dayStamp;

String timeStamp;

String path = "/Paso de Peatones";

int contador = 1;
bool enviar = false;

void setup()
{
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  Serial.begin(115200); // Enviaremos la información de depuración a través del Monitor de Serial
  pinMode(luz, OUTPUT);
  pinMode(presion, INPUT);
  digitalWrite(luz, LOW);

 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}
void loop()
{
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }
  

  fecha = timeClient.getFormattedDate();
  //int dia_semana = weekDays[timeClient.getDay()];
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);

  uint8_t day = ti->tm_mday;
  int dia = static_cast<int>(day);
  

  
  if(saltar == 0){
  
  unsigned long tiempo = millis();

  if(digitalRead(presion) == HIGH){
    digitalWrite(luz, HIGH);
    veces = veces +1;
    Serial.print("Veces = ");
    Serial.println(veces);
    millisPrev = tiempo;
    saltar = 1;

  }else{
    digitalWrite(luz, LOW);
  }
  }else{
    unsigned long tiempo = millis();
    if(tiempo - millisPrev >= tiempocruzar){
      saltar = 0;
    }

  }
   if((saltar != comprobar) && (saltar == 1)){
    enviar = true;
    comprobar = saltar;
   
  }else{
    enviar = false;
    comprobar = saltar;

  }

  if (Firebase.ready() && signupOK && enviar == true){
    json.add("Fecha", fecha);
    json.add("Dia", dia);
    if (Firebase.RTDB.pushJSON(&fbdo, path, &json)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.print("PUSH NAME: ");
      Serial.println(fbdo.pushName());
      Serial.println("ETag: " + fbdo.ETag());
      Serial.println("------------------------------------");
      Serial.println();
    }

    else
    {

      Serial.println("FAILED");

      Serial.println("REASON: " + fbdo.errorReason());

      Serial.println("------------------------------------");

      Serial.println();

    }
  }
}