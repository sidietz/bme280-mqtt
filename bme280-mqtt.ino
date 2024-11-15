#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "secrets.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define ALTITUDE 458
#define KELVIN 273.15
#define e 2.71828182846

Adafruit_BME280 bme;

//#define DHTTYPE DHT22
//DHT dht(D1, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


int chk;
float humidity;  //Stores humidity value
float temperature; //Stores temperature value
float pressure; //Stores pressure value
float seaPressure; //Stores sea level pressure

char ssid[] = "FRITZ!Box 7490_Legacy";    // your network SSID (name)
const char* serverName = "esp8266";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.178.64";
int port = 1883;

const char topic[]  = "home/sensors/bme280";

const long interval = 1000 * 60;
unsigned long previousMillis = 0;
unsigned long counter = 0;

float calcPsat(float temperature) {
  if (temperature > 0) {
    return 610.5 * pow(e, (17.269 * temperature / (237.3 + temperature)));
  } else {
    return 610.5 * pow(e, (21.875 * temperature / (265.5 + temperature)));
  }
}

float calcAltE(float humidity, float temperature) {
 return humidity * calcPsat(temperature);
}
// formular from:
// https://www.wufi-forum.com/viewtopic.php?t=1615
// https://enbau-online.ch/bauphysik/9-9-saettigungsdampfmenge-und-wasserdampfsaettigungsdruck/

float calcE(float humidity, float temperature)  {
  if (temperature < 0) {
    return (4.689 * pow((1.486 + temperature / 100), 12.3) * humidity) / 100;
  } else {
    return (288.68 * pow((1.0981 + temperature / 100), 8.02) * humidity) / 100;
  }

}

float guessE(float temperature) {
  if (temperature < 9.1) {
    return 5.6402 * (-0.0916 + pow(e, 0.06*temperature));
  } else {
    return 18.2194 * (1.0463 - pow(e, -0.0666*temperature));
  }
}

float convToSeaLevelPressure(float pressure, float temperature, float humidity) {
  float g_0 =  9.80665;
  float R = 287.05;
  float C_h = 0.12;
  float a = 0.0065;
  float E = calcE(humidity, temperature);
  Serial.println(E);
  Serial.println(guessE(temperature));

  float x = (g_0 / (R * ((temperature + KELVIN) + C_h * E + a * (ALTITUDE/2)))) * ALTITUDE;
  return pressure * pow(e, x);
}

float convToSeaLevelPressureEstimate(float pressure, float temperature) {
  //return pressure / pow((1 - (ALTITUDE / 44330)), 5.255); // BMP180 does not work
  //return 44330.0 * (1.0 - pow(pressure / SEALEVELPRESSURE_HPA, 0.1903));
  /*
  float g_0 =  9.80665;
  float R = 287.05;
  float C_h = 0.12;
  float a = 0.0065;
  float E = calcE(humidity, temperature);
  Serial.println(E);

  float x = (g_0 / (R * ((temperature + KELVIN) + C_h * E + a * (ALTITUDE/2)))) * ALTITUDE;

  return pressure * pow(e, x);
  */

  return ((pressure)/pow((1-((float)(ALTITUDE))/44330), 5.255));

  //return pressure * pow(temperature / (temperature + 0.0065 * ALTITUDE ), -5.255); // BMP180 does not work

  //return SEALEVELPRESSURE_HPA * pow((1 - (0,0065 * ALTITUDE) / (temperature + KELVIN)), 5.255);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.write("init: ");
  //dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.hostname(serverName);
  bool failFlag = true;
  int retryCount = 0;
  wl_status_t ws = WiFi.status();

  while (ws != WL_CONNECTED) {
    if(failFlag) {
      WiFi.begin(ssid, pass);
      failFlag = false;
    }
    Serial.print(".");
    delay(500);
    ws = WiFi.status();
    if(ws == WL_CONNECT_FAILED) {
      failFlag = true;
      delay(500);
      if(++retryCount > 100) {
        // We've done our best. Stopping
        Serial.println("");
        Serial.println("Connection failed permanently.");
      }
      Serial.println("");
      Serial.println("Connection failed. Retrying ...");
    }
  }
  Serial.println("");

  mqttClient.setUsernamePassword("simon", "N0m1596.");

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  bool status;

  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("BME280 sensor status ok!");

}

void loop() {
  mqttClient.poll();

  unsigned long currentMillis = millis();
  char humidityStr[8];
  char temperatureStr[8];
  char pressureStr[8];
  String msg;
  // put your main code here, to run repeatedly:
  //Serial.write("Hello world!");

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;


    humidity = bme.readHumidity();
    temperature = bme.readTemperature();
    pressure = bme.readPressure() / 100.0F; //convert to hPa
    seaPressure = convToSeaLevelPressure(pressure, temperature, humidity / 100); //conv percentage to float
    float seaPressure2 = convToSeaLevelPressureEstimate(pressure, temperature);
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Temp: ");
    Serial.print(temperature);
    Serial.println("° Celsius");
    Serial.print(pressure);
    Serial.println(" Pressure");
    Serial.print(seaPressure);
    Serial.print("hPa Pressure at sealevel vs.");
    Serial.println(seaPressure2);

    dtostrf(temperature, 2, 2, temperatureStr);
    dtostrf(humidity, 3, 2, humidityStr);
    dtostrf(seaPressure, 5, 2, pressureStr);


    Serial.print("Sending message to topic: ");
    Serial.println(topic);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    msg = String(temperatureStr) + String(',') + String(humidityStr) + String(',') + String(pressureStr);
    mqttClient.print(msg);
    mqttClient.endMessage();
    Serial.print("message: ");
    Serial.println(msg);
    counter++;
    Serial.println(counter);
  }
}
