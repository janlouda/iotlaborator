#include <SPI.h>
#include <EthernetClient.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <DHT.h>



// piny pro připojení Trig a Echo z modulu ultrazvuku
int pTrig = 4;
int pEcho = 5;
// pin DHT senzoru
int pDHT = 6;
// piny pro připojení RGB diody
int red = 7;
int green = 8;
int blue = 9;
// pin pro připojení serva
int pServo = 3;

/*
 * WATSON IOT FOUNDATION PARAMETRY ===============================================================
 * org - organization ID
 * type - device type
 * deviceid - device ID
 * token - autentization token
 */
char server[] = "3zaay5.messaging.internetofthings.ibmcloud.com";
int port = 1883;
const char evtTopic[] = "iot-2/evt/measure/fmt/json";
const char cmdTopic[] = "iot-2/cmd/listen/fmt/txt";
const char clientName[] = "d:3zaay5:ArduinoUno:arduino-iotlab";
char token[] = "I@Tlab76";

/* 
 *  ================================================================================================
*/

int publishInterval = 10000; //10 seconds 
long lastPublishMillis;


// inicializace proměnných, do kterých se uloží data
long odezva, vzdalenost;

//nastavení typu čidla DHT
#define typDHT11 DHT11

//nastvení Ethernet Shieldu a SD karty
#define ETH_CS  10
#define SD_CS  2
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0xBF, 0x0C};
char macstr[] = "90A2DA10BF0C";
float teplota; //inicializace proměnné teplota, kam se ukládá naměřená teplota
float vlhkost;  //inicializace proměnné vlhkost, kam se ukládá naměřená vlhkost
String deviceEvent; //proměnná do které se skládá json


/*
 * FUNKCE CALLBACKU S ČEKÁNÍM NA PŘÍKAZ K OTOČENÍ SERVA =====================================
 */

void cmdCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Command received [");
  Serial.print(topic);
  Serial.print("] ");

//  if (len < 2) { //bad message
//    free(payloadStr);
//    return;
//    } 

  char cmd = (char)payload[0];
  char cmd_value = (char)payload[1];

if (cmd == 's' || cmd == 'S') { //Servo
    if (cmd_value == 'o' || cmd_value == 'O'){
      servo_open();
      Serial.println("Servo opened.");
    }
    if (cmd_value == 'c' || cmd_value == 'C'){
      servo_close();
      Serial.println("Servo closed.");
    }
    return;
  }
if (cmd == 'l' || cmd == 'L') {
  if (cmd_value == 'r' || cmd_value == 'R'){
      shine_red();
      Serial.println("Red light.");
    }
    if (cmd_value == 'g' || cmd_value == 'G'){
      shine_green();
      Serial.println("Green light.");
    }
    if (cmd_value == 'b' || cmd_value == 'B'){
      shine_blue();
      Serial.println("Blue light.");
    }
   if (cmd_value == 'o' || cmd_value == 'O'){
      shine_off();
      Serial.println("Light OFF.");
    }
    return;
  }
}


// inicializace periférií
DHT myDHT(pDHT, typDHT11);
Servo myServo; 
EthernetClient ethClient;
PubSubClient pubSubClient(server, port, cmdCallback, ethClient);

/*
 * FUNKCE PŘIPOJENÍ NA WATSON IOT PLATFORMU ===========================
 */
void connectToIoTP(){
  
  if (pubSubClient.connect(clientName, "use-token-auth", token)) {
    Serial.println("Connection to MQTT Broker Successfull");
    if (pubSubClient.subscribe(cmdTopic)){
      Serial.println("Subcription OK");
    } else {
      Serial.println("Subcription FAILED");
    }

  } else {
    Serial.println("Connection to MQTT Broker Failed");
  }
}


/*
 * FUNKCE OTOČENÍ SERVA ===============================================
 */

void servo_open(){
  myServo.write(185);
}

void servo_close(){
  myServo.write(90);
}

/*
 * FUNKCE ROZSVĚCENÍ DIODY ===============================================
 */

void shine_red(){
  digitalWrite(7, HIGH);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
}

void shine_green(){
  digitalWrite(7, LOW);
  digitalWrite(8, HIGH);
  digitalWrite(9, LOW);

}

void shine_blue(){
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, HIGH);

}

void shine_off(){
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);

}


void setup() {
// nastavení pinů ultrazvuk modulu jako výstup a vstup
  pinMode(pTrig, OUTPUT);
  pinMode(pEcho, INPUT);
//nastavení pinu DHT na výstup
  pinMode(pDHT,OUTPUT);
// nastavení pinů RGB diody jako výstup
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
// piny ethernet shieldu a SD karty
  pinMode(ETH_CS,OUTPUT);
  pinMode(SD_CS,OUTPUT);
  digitalWrite(ETH_CS,LOW); //select the Ethernet Module
  digitalWrite(SD_CS,HIGH); //de-Select the internal SD Card
// zapnutí komunikace s teploměrem DHT
  myDHT.begin();
// zapnutí komunikace se servem 
  myServo.attach(pServo);
// výpis na konzoli
  Serial.begin(9600);
  Serial.println("Začínám připojení.");
  Ethernet.begin(mac);
  Serial.println(Ethernet.localIP());
//připojení na IoT PLatformu
  connectToIoTP();

}

void loop() {

    if (millis() - lastPublishMillis > publishInterval){

  // ČTENÍ VZÁLENOSTI Z ULTRAZVUKOVÉHO ČIDLA
  
      //vysílání signálu
      digitalWrite(pTrig, LOW);
      delayMicroseconds(2);
      digitalWrite(pTrig, HIGH);
      delayMicroseconds(5);
      digitalWrite(pTrig, LOW);
    
      // pomocí funkce pulseIn získáme následně
      // délku pulzu v mikrosekundách (us)
      odezva = pulseIn(pEcho, HIGH);
      // přepočet získaného času na vzdálenost v cm
      vzdalenost = odezva / 58.31;
      Serial.print("Vzdalenost je ");
      Serial.print(vzdalenost);
      Serial.println(" cm.");
      
 
 // ČTENÍ TEPLOTY A VLHKOSTI Z ČIDLA DHT11  

        teplota = myDHT.readTemperature();
        vlhkost = myDHT.readHumidity();
        Serial.print("Teplota je ");
        Serial.print(teplota);
        Serial.println(" °C.");
        Serial.print("Vlhkost je ");
        Serial.print(vlhkost);
        Serial.println(" %.");

    //Build JSON
    String json = createJSONMessage();
    char jsonStr[100];
    json.toCharArray(jsonStr,100);


    //Send JSON to Watson IoT Platform
    if (pubSubClient.connected()){
   
      boolean pubresult = pubSubClient.publish(evtTopic,jsonStr);
      Serial.print("Sending ");
      Serial.println(jsonStr);
      Serial.print("to ");
      Serial.println(evtTopic);
      if (pubresult) {
        Serial.println("SUCCESS\n");
      } else {
        Serial.println("NOT SUCCESSFULL\n");
      }
    }
 
    lastPublishMillis = millis();
    }

   if (!pubSubClient.loop()) {
   connectToIoTP();
 }
}

//Create a JSON message 
String createJSONMessage() {
  String data = "{\"d\":{\"TEMP\":";
  data += (float)teplota;
  data += ",\"HUMI\":";
  data += (float)vlhkost; 
  data += ",\"DIST\":";
  data += (float)vzdalenost; 
  data += "}}";
  return data;
}


