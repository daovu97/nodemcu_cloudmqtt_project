#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>
#include <PubSubClient.h>

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "BKITC"
#define WLAN_PASS       "2444666668888888"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "daovu"
#define AIO_KEY         "3a7e777c10db44d08bfa7a3fbcad0adb"

/*************************pin Setup *********************************/
const int RL1pin = 14;
const int RL2pin = 12;
const int RL3pin = 13;
const int RL4pin = 15;
const int ON = HIGH;
const int OFF = LOW;

/*************************DHT Setup *********************************/
float humidity;
float temperature;
unsigned long readTime;
const int DHTpin = 4;         //đọc data từ chân  gpio4
const int DHTtype = DHT11;    //khai báo loại cảm biến
DHT dht(DHTpin, DHTtype); //khởi tạo dht

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/TEMPERATURE");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/HUMIDITY");

// Setup a feed for subscribing to changes.
Adafruit_MQTT_Subscribe RL1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/RL1");
Adafruit_MQTT_Subscribe RL2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/RL2");
Adafruit_MQTT_Subscribe RL3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/RL3");
Adafruit_MQTT_Subscribe RL4 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/RL4");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup() {
  WiFi.disconnect();
  Serial.begin(9600);
  dht.begin();   //khởi động cảm biến
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());


  //set pinmode
  pinMode(RL1pin, OUTPUT);
  pinMode(RL2pin, OUTPUT);
  pinMode(RL3pin, OUTPUT);
  pinMode(RL4pin, OUTPUT);

  //set bit first time
  digitalWrite(RL1pin, OFF);
  digitalWrite(RL2pin, OFF);
  digitalWrite(RL3pin, OFF);
  digitalWrite(RL4pin, OFF);

  // Setup MQTT subscription
  mqtt.subscribe(&RL1);
  mqtt.subscribe(&RL2);
  mqtt.subscribe(&RL3);
  mqtt.subscribe(&RL4);
}

uint32_t x = 0;
void sensorRead() {
  readTime = millis();
  humidity = dht.readHumidity(); //đọc nhiệt độ
  temperature = dht.readTemperature(); // đọc độ ẩm

  //kiểm tra sensor có hoạt động??
  if (isnan(humidity) || isnan(temperature) ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  //đẩy data lên server
  char buffer[10];
  dtostrf(temperature, 0, 0, buffer);
  Temperature.publish(buffer);

  dtostrf(humidity, 0, 0, buffer);
  Humidity.publish(buffer);
}
void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
 
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &RL1) {
      // convert mqtt ascii payload to int
      char *value = (char *)RL1.lastread;
      String message = String(value);
      message.trim();
      if (message == "ON") {
        digitalWrite(RL1pin, HIGH);
      }
      if (message == "OFF") {
        digitalWrite(RL1pin, LOW);
      }
    }

    if (subscription == &RL2) {
      // convert mqtt ascii payload to int
      char *value = (char *)RL2.lastread;
      String message = String(value);
      message.trim();
      if (message == "ON") {
        digitalWrite(RL2pin, HIGH);
      }
      if (message == "OFF") {
        digitalWrite(RL2pin, LOW);
      }
    }

    if (subscription == &RL3) {
      // convert mqtt ascii payload to int
      char *value = (char *)RL3.lastread;
      String message = String(value);
      message.trim();
      if (message == "ON") {
        digitalWrite(RL3pin, HIGH);
      }
      if (message == "OFF") {
        digitalWrite(RL3pin, LOW);
      }
    }

    if (subscription == &RL4) {
      // convert mqtt ascii payload to int
      char *value = (char *)RL4.lastread;
      String message = String(value);
      message.trim();
      if (message == "ON") {
        digitalWrite(RL4pin, HIGH);
      }
      if (message == "OFF") {
        digitalWrite(RL4pin, LOW);
      }
    }

  }

  if (millis() > readTime + 4000) {
    sensorRead();
  }
   
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
