/* Drafted by Jordan Mellish (jordan.mellish@townsville.qld.gov.au)
 * MIT License 
 * Refined by Tristan Fivaz (tristan.fivaz@townsville.qld.gov.au)
 * Description: Code for sending DHT11(Temperature and Humidity) and MQ7(Carbon Monoxide) data to ThingSpeak with a WeMos D1 Pro or D1 Mini  
 * Including ESP Deep Sleep for 15 minutes.
 */

/* Import Libraries  */ 
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <DHT.h>

/* Definitions */ 
#define DHTPIN D6 
#define DHTTYPE DHT11
#define analogMQ7 A0      // Signal  
#define ledPin D7         // Device internal LED       
DHT dht(DHTPIN, DHTTYPE);
const int sleepTimeS = 900;
int MQ7sensorValue = 0;   // value read from the sensor
int sensorvalue = 0;  // variable to store the value coming from the sensor

/* Wi-Fi Credentials */ 
char ssid[] = "CHANGE_ME";    //  your network SSID (name) 
char pswd[] = "CHANGE_ME";   // your network password
int status = WL_IDLE_STATUS;
WiFiClient  client;

/* ThingSpeak Credentials  */
unsigned long myChannelNumber1 = CHANGE_ME; // Enter your channel number from ThingSpeak
const char * myWriteAPIKey1 = "CHANGE_ME"; //Enter your Write API Key from ThingSpeak

void setup() {
  Serial.begin(115200); 
  pinMode(analogMQ7, INPUT);
  pinMode(ledPin, OUTPUT);
  dht.begin(); 
  ThingSpeak.begin(client);
  if (WiFi.status() != WL_CONNECTED) { connectWiFi();}
    Serial.print("I'm Awake");
    delay(2000);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
      // A) preparation
    // turn the heater fully on
    analogWrite(analogMQ7, HIGH); // HIGH = 255
    // heat for 1 min
    delay(60000);
    // now reducing the heating power: turn the heater to approx 1,4V
    analogWrite(analogMQ7, 71.4); // 255x1400/5000
    // heat for 90 sec
    delay(90000);
    
  // B) reading    
    // CO2 via MQ7: we need to read the sensor at 5V, but must not let it heat up. So hurry!
    analogWrite(analogMQ7, HIGH); 
    delay(50); // Getting an analog read apparently takes 100uSec
    MQ7sensorValue = analogRead(analogMQ7);          
    Serial.print(t);
    Serial.print(h);
    Serial.print(MQ7sensorValue);
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT Sensor");
      return;
    } else { 
    //ThingSpeak.setField(Field No. , data)
    ThingSpeak.setField(1, t);
    ThingSpeak.setField(2, h);
    ThingSpeak.setField(3, MQ7sensorValue);
    ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
        if (MQ7sensorValue <= 200) 
    {
        Serial.println("Air-Quality: CO perfect");
    }
    else if ((MQ7sensorValue > 200) || (MQ7sensorValue <= 800)) // || = or
    {
        Serial.println("Air-Quality: CO normal");
    }
    else if ((MQ7sensorValue > 800) || (MQ7sensorValue <= 1800))
    {
        Serial.println("Air-Quality: CO high");
    }
    else if (MQ7sensorValue > 1800) 
    {
        digitalWrite(ledPin, HIGH); // optical information in case of emergency
        Serial.println("Air-Quality: ALARM CO very high");
        delay(3000);
        digitalWrite(ledPin, LOW);
    }
    else
    {
        Serial.println("MQ-7 - cant read any value - check the sensor!");
    }
    delay(2000);
    
    Serial.println("Back to sleep");
    ESP.deepSleep(sleepTimeS * 1000000);
    }

}

void loop() {
}

void connectWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid,pswd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Address :");
  Serial.println(WiFi.localIP());
  Serial.println();
}
