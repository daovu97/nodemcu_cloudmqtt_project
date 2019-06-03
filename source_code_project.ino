#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/************************* WiFi Access Point *********************************/

//#define WLAN_SSID "MANDEVICES"
//#define WLAN_PASS "ptndoluong"

#define WLAN_SSID "DaoVu"
#define WLAN_PASS "1900100co_"

const int ON = HIGH;
const int OFF = LOW;
/************************* cloudmqtt Setup *********************************/

#define serveruri "m16.cloudmqtt.com"
#define port       11210
#define username  "ubukzoam"
#define password  "DK3eXPuJwnao"

/*************************pin Setup *********************************/
const int DHTpin = 4; //đọc data từ chân  gpio4
const int RL1pin = 14;
const int RL2pin = 12;
const int RL3pin = 13;
const int RL4pin = 15;

/*************************DHT Setup *********************************/
const int DHTtype = DHT11; //khai báo loại cảm biến
DHT dht(DHTpin, DHTtype);  //khởi tạo dht
float humidity;
float temperature;
unsigned long readTime;
unsigned long feedBackTime;

//tạo 1 client
WiFiClient myClient;

/*************************** Sketch Code ************************************/
void callback(char *tp, byte *message, unsigned int length);
void sensorRead();
void reconnect();
void feedBack();

//khởi tạo pubsubclient
PubSubClient mqtt(serveruri, port, callback, myClient);

void setup()
{
  WiFi.disconnect();
  Serial.begin(9600);
  dht.begin(); //khởi động cảm biến
  // Connect to WiFi access point.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // kết nối với mqtt server
  while (1)
  {
    delay(500);
    if (mqtt.connect("ESP8266", username, password))
      break;
  }
  Serial.println("connected to MQTT server.....");

  //nhận dữ liệu có topic "ESPn" từ server
  mqtt.subscribe("ESPn/RL1");
  mqtt.subscribe("ESPn/RL2");
  mqtt.subscribe("ESPn/RL3");
  mqtt.subscribe("ESPn/RL4");

  //set mode
  pinMode(DHTpin, INPUT);
  pinMode(RL1pin, OUTPUT);
  pinMode(RL2pin, OUTPUT);
  pinMode(RL3pin, OUTPUT);
  pinMode(RL4pin, OUTPUT);

  //set bit first time
  digitalWrite(RL1pin, OFF);
  digitalWrite(RL2pin, OFF);
  digitalWrite(RL3pin, OFF);
  digitalWrite(RL4pin, OFF);
}

void loop()
{
  if (WiFi.status() == WL_DISCONNECTED || !mqtt.connected())
  {
    reconnect();
  }

  //làm mqtt luôn sống
  mqtt.loop();

  //phản hồi trạng thái relay lên server
  if (mqtt.connected())
  {
    if (millis() > feedBackTime + 435)
    {
      feedBack();
    }


    //check if 5 seconds has elapsed since the last time we read the sensors.
    if (millis() > readTime + 5000)
    {
      sensorRead();
    }
  }
}

// hàm trả dữ liệu về
void callback(char *tp, byte *message, unsigned int length)
{

  String topic(tp);
  String content = String((char *)message);
  content.remove(length);

  //điều khiển relay 1
  if (topic == "ESPn/RL1")
  {
    if (content == "1")
    {
      digitalWrite(RL1pin, ON);
      Serial.println("relay 1 ON");
    }
    if (content == "0")
    {
      digitalWrite(RL1pin, OFF);
      Serial.println("relay 1 OFF");
    }
  }

  //điều khiển relay 2
  if (topic == "ESPn/RL2")
  {
    if (content == "1")
    {
      digitalWrite(RL2pin, ON);
      Serial.println("relay 2 ON");
    }
    if (content == "0")
    {
      digitalWrite(RL2pin, OFF);
      Serial.println("relay 2 OFF");
    }
  }

  //điều khiển relay 3
  if (topic == "ESPn/RL3")
  {
    if (content == "1")
    {
      digitalWrite(RL3pin, ON);
      Serial.println("relay 3 ON");
    }
    if (content == "0")
    {
      digitalWrite(RL3pin, OFF);
      Serial.println("relay 3 OFF");
    }
  }

  //điều khiển relay 4
  if (topic == "ESPn/RL4")
  {
    if (content == "1")
    {
      digitalWrite(RL4pin, ON);
      Serial.println("relay 4 ON");
    }
    if (content == "0")
    {
      digitalWrite(RL4pin, OFF);
      Serial.println("relay 4 OFF");
    }
  }
}

// hàm đọc giá trị sensor
void sensorRead()
{
  readTime = millis();
  humidity = dht.readHumidity();       //đọc nhiệt độ
  temperature = dht.readTemperature(); // đọc độ ẩm

  //kiểm tra sensor có hoạt động??
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Serial.print(humidity);
  // Serial.println("%");
  // Serial.print(temperature);
  // Serial.println("độ c");
  //đẩy data lên server
  char buffer[10];
  dtostrf(temperature, 0, 0, buffer);
  mqtt.publish("ESP/temperature", buffer);

  dtostrf(humidity, 0, 0, buffer);
  mqtt.publish("ESP/humidity", buffer);
}

//hàm reconnect
void reconnect()
{
  // lặp đến khi kết nối lại
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Attempting connection...");
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    mqtt.connect("ESP8266", username, password);
    delay(500);
    // chờ để kết nối lại
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("reconnected");
      return;
    }
    else
    {
      Serial.print("failed to connect WiFi!!");
      Serial.println(" try again in 5 seconds...");
      // chờ 5s
      delay(5000);
    }
  }

  while (!mqtt.connected())
  {
    Serial.println("Attempting connection...");
    mqtt.connect("ESP8266", username, password);
    delay(500);
    // chờ để kết nối lại
    if (mqtt.connected())
    {
      Serial.println("reconnected");
      mqtt.subscribe("ESPn/RL1");
      mqtt.subscribe("ESPn/RL2");
      mqtt.subscribe("ESPn/RL3");
      mqtt.subscribe("ESPn/RL4");
      return;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // chờ 5s
      delay(5000);
    }
  }
}

void feedBack() {
  feedBackTime = millis();
  mqtt.publish("ESPg/RL1", String(digitalRead(RL1pin)).c_str());
  mqtt.publish("ESPg/RL2", String(digitalRead(RL2pin)).c_str());
  mqtt.publish("ESPg/RL3", String(digitalRead(RL3pin)).c_str());
  mqtt.publish("ESPg/RL4", String(digitalRead(RL4pin)).c_str());
}
