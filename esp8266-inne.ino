/*
 *  Simple HTTP get webclient test
 */
 
#include <ESP8266WiFi.h>

#include "DHT.h"        // DHT22 temperature and humidity sensor
#define DHTPIN 5
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = "tupsu";
//const char* password = "";
WiFiServer server(80);

unsigned long time = 0;
unsigned long serialtime = 0;
boolean ledon = true;
 
void setup() {
  pinMode(A0, INPUT);
  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(16, LOW);
  Serial.begin(115200);
  delay(10);
 
  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  
}
 
int value = 0;
 
void loop() {
  
  WiFiClient client = server.available();

  if(client) {
    digitalWrite(2, LOW); ledon=true;
    time = millis();
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if(client.available()) {
        char c = client.read();
        if (c=='\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain; charset=utf-8");
          client.println("Connection: close");
          client.println();
          client.println("{");
          client.print("  \"temperature\":");
          client.print(getTemp());
          client.println(",");
          client.print("  \"humidity\":");
          client.print(getHum());
          client.println(",");
          client.print("  \"brightness-raw\":");
          client.println(getBrightness());
          client.println("}");
          break;
        }
        if (c=='\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }

  if(ledon && millis()-time>1000) {
    digitalWrite(2, HIGH);
    digitalWrite(16, HIGH);
    ledon = false;
  }

  if(millis()-serialtime>1000) {
    Serial.println( analogRead(A0) );
    serialtime=millis();
  }
  
}


float getTemp() {
  float t = dht.readTemperature();
  Serial.print("Temp: ");
  Serial.print(t);
  return t;
}

float getHum() {
  float h = dht.readHumidity();
  Serial.print(", Humidity: ");
  Serial.print(h);
  return h;
}

int getBrightness() {
  int b = analogRead(A0);
  Serial.print(", Brightness: ");
  Serial.println(b);
  return b;
}

