

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
#include <PubSubClient.h>      // Biblioteca de MQTT

#include "string.h"

//definir SENSOR PRIMER TRAMO
#define sensor1 5  //GPIO
#define sensor2 4

//definir SENSOR SEGUNDO TRAMO
#define sensor3 13
#define sensor4 A0 

//definir pines SEMAFORO 1
#define ROJO1 14
#define NARANJA1 2
#define VERDE1 0

//definir pines SEMAFORO 2
#define ROJO2 15
#define NARANJA2 16
#define VERDE2 12

// Insert your network credentials
#define WIFI_SSID "DIGIFIBRA-c54Q"
#define WIFI_PASSWORD "94GuTYXQeT"

//-----------------------------------Firebase------------------------------------
// Insert Firebase project API Key
#define API_KEY "AIzaSyAaua17IgfeQY6Ji7svpyCofhWc-oecwqI"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://basededatos-f1968-default-rtdb.europe-west1.firebasedatabase.app"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


//-----------------------------------MQTT------------------------------------


const char* ssid = "DIGIFIBRA-c54Q";                    // Nombre de la red WiFi
const char* password = "94GuTYXQeT";               // Contraseña de la red WiFi
// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";    // Dirección IP o nombre de dominio del broker MQTT
const int mqtt_port = 1883;                    // Puerto del broker MQTT
const char *mqtt_username = "emqx";            // Usuario para autenticación (si es necesario)
const char *mqtt_password = "public";          // Contraseña para autenticación (si es necesario)
const char *publishTopic = "arduinoUAH2023/prueba";   // Tópico MQTT para publicar mensajes
const char *subscribeTopic  = "arduinoUAH2023/prueba2"; //Topico MQTT del que recibe los mensajes

WiFiClient espClient;
PubSubClient client(espClient);

//-----------------------------------VARIABLES GLOBALES------------------------------------

String mensajeRecibido; // Variable global para almacenar el mensaje recibido
//Se inicializan los colores de los semaforos. 
//El semaforo1 empieza en verde y el segundo semaforo en rojo

String ledApagar = "";
bool signupOK = false;
String fecha;
///Week Days
int weekDays[7]={6, 0, 1, 2, 3, 4, 5};
String path2 = "/Semaforos";

// Tiempo de duración de cada estado (en milisegundos)
int tiempoSemaforo = 3000;
// Variable para realizar un seguimiento del estado actual
int estado = 0;
//Variable para calcular en numero de coches
int coches1 = 0;
int coches2 = 0;

String color1 = "";
String color2 = "";
char buffer1[10];  // Buffer para almacenar la cadena resultante
char buffer2[10];  // Buffer para almacenar la cadena resultante
int contador = 0;
bool enviado = true;

int paso = 0;
unsigned long millisIniciales = 0;
unsigned long millisIniciales2 = 0;



//Recibir un mensaje 
void callback(char* topic, byte* payload, unsigned int length) {
  // Almacena el mensaje recibido en la variable global
  mensajeRecibido = "";
  for (int i = 0; i < length; i++) {
    mensajeRecibido += (char)payload[i];
  }
  enviado = true;
  
}

//Reconecta el programa al topico en caso de que se desconecte
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ArduinoClient")) {
      Serial.println("Conectado");
      client.subscribe(subscribeTopic);
    } else {
      Serial.print("Error de conexión MQTT: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

//-----------------------------------SETUP------------------------------------

void setup() {
  //Luces de los semaforos
  //Luces SEMAFORO1
  pinMode(ROJO1, OUTPUT);
  pinMode(NARANJA1, OUTPUT);
  pinMode(VERDE1, OUTPUT);
  //Luces SEMAFORO2
  pinMode(ROJO2, OUTPUT);
  pinMode(NARANJA2, OUTPUT);
  pinMode(VERDE2, OUTPUT);

  //sensores de presion
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT);
  //inicializar semaforos

  digitalWrite(VERDE1, HIGH);
  digitalWrite(ROJO2, HIGH);

  //Conectandose al Firebase
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Assign the api key (required) 
  config.api_key = API_KEY;

  // Assign the RTDB URL (required) 
  config.database_url = DATABASE_URL;

  // Sign up 
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task 
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  //Conectandose al MQTT
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }
  
  Serial.println("Conexión WiFi establecida");
  
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
  while (!client.connected()) {
    Serial.println("Conectando al broker MQTT...");
    
    if (client.connect("esp8266Client", mqtt_username, mqtt_password)) {
      Serial.println("Conexión MQTT establecida");
    } else {
      Serial.print("Fallo en la conexión MQTT, estado: ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  client.subscribe(subscribeTopic); //Se suscribe al topic del que va a recibir los mensajes
}



//-----------------------------------LOOP------------------------------------
void loop() {

      


  fecha = timeClient.getFormattedDate();
  
  timeClient.update();
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);

  uint8_t day = ti->tm_mday;
  int dia = static_cast<int>(day);
  

  

  //----------------------comprobar 1 ------------------------------------
  int comprobar1 = digitalRead(sensor1);
  int comprobar2 = digitalRead(sensor2);
  //Si el primer sensor se activa
  if(comprobar1 == HIGH){
    Serial.println("Se ha Pulsado el SENSOR 1 del TRAMO 1");
    coches1++ ;//Se suma 1 a los coches que pasan 
      //añade la informacion a un JSON
    json.add("Fecha", fecha);
    json.add("Dia", dia);
    json.add("Coches", 1);
    json.add("Tramo", 1);
    //Se envia a Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, path2, &json)){
      Serial.println("------------------------------------");
      Serial.println();
    }else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
  if (comprobar2 == HIGH){
    Serial.println("Se ha Pulsado el SENSOR 2 del TRAMO 1");
    coches1--;
  }
  if (coches1 < 0) {
    coches1 = 0;
  }

  //----------------------comprobar 2 ------------------------------------
  int comprobar3 = digitalRead(sensor3);
  int comprobar4 = analogRead(sensor4);

  //Si el primer sensor se activa
  if(comprobar3 == HIGH){
    Serial.println("Se ha Pulsado el SENSOR 1 del TRAMO 2");
    coches2++; //Se suma 1 a los coches que pasan 
      //añade la informacion a un JSON
    json.add("Fecha", fecha);
    json.add("Dia", dia);
    json.add("Coches", 1);
    json.add("Tramo", 2);
    //Se envia a Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, path2, &json)){
      Serial.println("------------------------------------");
      Serial.println();
    }else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
  if (comprobar4 > 400){
    Serial.println("Se ha Pulsado el SENSOR 2 del TRAMO 2");
    coches2--;
  }
  if (coches2 < 0) {
    coches2 = 0;
  }

  if(paso == 0){
    contador++;
    Serial.println(contador);
    if (enviado){
      enviarCoches1(dia);
      enviarCoches2(dia);
      paso = 1;
      millisIniciales = millis();
    }
  }
  unsigned long millisActuales = millis();
  if((paso == 1) && ((millisActuales - millisIniciales) >= 3000)){
    paso = 2;
  }
  
  if(paso == 2){  
    client.loop();
    // Si se ha mensaje significa que habido placas presionadas, por lo cual -> Trafico
    if (mensajeRecibido != "") {
      if (mensajeRecibido == "igual"){
        //Si es igual signfica que hay el mismo numero de coches por lo que se hace de forma secuencial
        Serial.println("Algoritmo Predeterminado");
        algoritmoPredeterminado();
      }else{
        Serial.print("[MQTT] Mensaje Recibido: ");
    
        color1 = mensajeRecibido.substring(0, 2);
        color2 = mensajeRecibido.substring(2);
        
        Serial.println(color1);
        Serial.print("y ");
        Serial.println(color2);
        

        //Secuencia de colores del Primer semaforo
        if (color1 == "v1") {
          digitalWrite(VERDE1, LOW);
          digitalWrite(NARANJA1, LOW);
          digitalWrite(ROJO1, LOW);
          digitalWrite(VERDE1, HIGH);
          Serial.println("Enciendo VERDE1");
          
        } if (color1 == "r1") {
          digitalWrite(VERDE1, LOW);
          digitalWrite(NARANJA1, LOW);
          digitalWrite(ROJO1, LOW);
          digitalWrite(ROJO1, HIGH);
          Serial.println("Enciendo ROJO1");
            
      
        }if (color1 == "a1") {
            digitalWrite(VERDE1, LOW);
            digitalWrite(NARANJA1, LOW);
            digitalWrite(ROJO1, LOW);
            digitalWrite(NARANJA1, HIGH);
            Serial.println("Enciendo NARANJA1");
          
        }
        //Secuencia de colores del Segundo semaforo
        if (color2 == "v2") {
          digitalWrite(VERDE2, LOW);
          digitalWrite(NARANJA2, LOW);
          digitalWrite(ROJO2, LOW);
          digitalWrite(VERDE2, HIGH);
          Serial.println("Enciendo verde2");
          
        }  if (color2 == "r2") {
          digitalWrite(VERDE2, LOW);
          digitalWrite(NARANJA2, LOW);
          digitalWrite(ROJO2, LOW);
          digitalWrite(ROJO2, HIGH);
          Serial.println("Enciendo ROJO2");
            
      
        } if (color2 == "a2") {
            digitalWrite(VERDE2, LOW);
            digitalWrite(NARANJA2, LOW);
            digitalWrite(ROJO2, LOW);
            digitalWrite(NARANJA2, HIGH);
            Serial.println("Enciendo NARANJA2");
          
        }
        mensajeRecibido = ""; // Reinicia la variable después de trabajar con el mensaje
      }   
    }
    millisIniciales2 = millis();
    paso = 3;
  }
  unsigned long millisActuales2 = millis();
  if((paso == 3) && ((millisActuales2 - millisIniciales2) >= 6000)){
    paso = 0;
  }
  delay(1000);
}

//Se envian al MQTT los coches que hay en el primer Tramo
void enviarCoches1(int dia_semana){
  
  int comprobar1 = digitalRead(sensor1);
  int comprobar2 = digitalRead(sensor2);
  //Si el primer sensor se activa
  if(comprobar1 == HIGH){
    Serial.println("Se ha Pulsado el SENSOR 1 del TRAMO 1");
    coches1++ ;//Se suma 1 a los coches que pasan 
      //añade la informacion a un JSON
    json.add("Fecha", fecha);
    json.add("Dia", dia_semana);
    json.add("Coches", 1);
    json.add("Tramo", 1);
    //Se envia a Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, path2, &json)){
      Serial.println("------------------------------------");
      Serial.println();
    }else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
  if (comprobar2 == HIGH){
    Serial.println("Se ha Pulsado el SENSOR 2 del TRAMO 1");
    coches1--;
  }
  if (coches1 < 0) {
    coches1 = 0;
  }
  itoa(coches1, buffer1, 10);  // Convertir el número entero a cadena
  const char* cadena = buffer1;  // Asignar la cadena a un puntero const char*
  // Envía un mensaje MQTT al tópico especificado con el contenido deseado
  client.publish(publishTopic, cadena);
  Serial.println("[MQTT] Se envia la informacion al MQTT");
  Serial.println("En el tamo 1 hay ");
  Serial.println(coches1);
  Serial.println("-----------------------");
  delay(100);  // Espera 0.1 segundos antes de enviar el siguiente mensaje

  

}

//Se envian al MQTT los coches que hay en el segundo Tramo
void enviarCoches2(int dia_semana){
  
  int comprobar1 = digitalRead(sensor3);
  int comprobar2 = analogRead(sensor4);

  //Si el primer sensor se activa
  if(comprobar1 == HIGH){
    Serial.println("Se ha Pulsado el SENSOR 1 del TRAMO 2");
    coches2++; //Se suma 1 a los coches que pasan 
      //añade la informacion a un JSON
    json.add("Fecha", fecha);
    json.add("Dia", dia_semana);
    json.add("Coches", 1);
    json.add("Tramo", 2);
    //Se envia a Firebase
    if (Firebase.RTDB.pushJSON(&fbdo, path2, &json)){
      Serial.println("------------------------------------");
      Serial.println();
    }else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
  }
  if (comprobar2 > 400){
    Serial.println("Se ha Pulsado el SENSOR 2 del TRAMO 2");
    coches2--;
  }
  if (coches2 < 0) {
    coches2 = 0;
  }
  itoa(coches2, buffer2, 10);  // Convertir el número entero a cadena

  const char* cadena = buffer2;  // Asignar la cadena a un puntero const char*
  // Envía un mensaje MQTT al tópico especificado con el contenido deseado
  client.publish(publishTopic, cadena);
  Serial.println("[MQTT] Se envia la informacion al MQTT");
  Serial.println("En el tamo 2 hay ");
  Serial.println(coches2);
  Serial.println("-----------------------");
  delay(100);  // Espera 0.1 segundos antes de enviar el siguiente mensaje

 
}

void algoritmoPredeterminado(){
  digitalWrite(VERDE1, LOW);
  digitalWrite(NARANJA1, LOW);
  digitalWrite(ROJO1, LOW);
  digitalWrite(VERDE2, LOW);
  digitalWrite(NARANJA2, LOW);
  digitalWrite(ROJO2, LOW);

  // Encender el LED correspondiente al estado actual
  if (estado == 0) {
    digitalWrite(VERDE1, HIGH);
    digitalWrite(ROJO2, HIGH);
  } else if (estado == 1) {
    digitalWrite(NARANJA1, HIGH);
    digitalWrite(ROJO2, HIGH);
  } else if (estado == 2) {
    digitalWrite(ROJO1, HIGH);
    digitalWrite(VERDE2, HIGH);
  }
  else if (estado == 3) {
    digitalWrite(ROJO1, HIGH);
    digitalWrite(NARANJA2, HIGH);
  }

  // Incrementar el estado
  estado++;

  // Si el estado alcanza 3, reiniciar a 0
  if (estado == 4) {
    estado = 0;
  }

  // Esperar el tiempo de encendido antes de pasar al siguiente LED
  //delay(tiempoSemaforo);
      
}



