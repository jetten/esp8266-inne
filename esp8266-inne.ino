/*
 *  Simple HTTP get webclient test
 */
 
#include <ESP8266WiFi.h>

#include "DHT.h"        // DHT22 temperature and humidity sensor
#define DHTPIN 5
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

/*#include <OneWire.h>
OneWire ds(13); // Dallas temp sensor on D7*/

const char welcomeMessage[] PROGMEM = {
  "<!DOCTYPE HTML>\n"
  "<html>"
  "<head>"
    "<title>ESP8266 environmental data logger</title>"
    "<link rel=\"stylesheet\" href=\"//jiihon.com/temp/this_server/css/default.css\" type=\"text/css\" />"
    "<link rel=\"shortcut icon\" href=\"//jiihon.com/favicon.ico\" />"
    "<style>"
      "body {font-size: 1.0em;}"
      "pre {display: inline;}"
      "h2 {margin-bottom: 0px;}"
      "p {margin-top: 0px;}"
    "</style>"
  "</head>"
  "<body>"
  "<p>This is the NodeMCU ESP8266 IoT climate sensor."
  "Cached data is available from <a href=\"//inne.jiihon.com\">inne.jiihon.com</a> if you prefer.</p>"
  "<p><img src=\"//inne.jiihon.com/nodemcu-sensor.jpg\"/></a>"
  "<h2>API access</h2>"
  "<p>Access the API from <a href=\"/inne\">/inne</a></p>"
  "<h2>API documentation</h2>"
  "<p>"
    "The data from the api is in JSON format<br>"
    "<pre>temperature:</pre> Inside temperature in C<br>"
    "<pre>humidity:</pre> Humidity in %RH<br>"
    "<pre>brightness-raw:</pre> Raw brightness value, range 0-1023. A value <= 16 usually means that lights are off. Values > 100 means daylight."
  "</p>"
};

const char* ssid     = "tupsu";
//const char* ssid     = "aalto open";
//const char* password = "";
WiFiServer server(80);

unsigned long time = 0;
unsigned long serialtime = 0;
boolean ledon = true;
 
void setup() {
  /*pinMode(12,OUTPUT); digitalWrite(12,LOW);// Dallas VDD D6
  pinMode(15,OUTPUT); digitalWrite(15,HIGH); // Dallas GND D8 */
    
  pinMode(A0, INPUT);
  pinMode(2, OUTPUT); //Wifi module led
  pinMode(16, OUTPUT); //board led
  digitalWrite(2, HIGH);
  digitalWrite(16, LOW);
  Serial.begin(115200);
 
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

    String request = client.readStringUntil('\r');
    client.flush();
    yield();

    client.println("HTTP/1.1 200 OK");
    if(request.indexOf("inne") != -1) {
      client.println("Content-type: text/plain; charset=utf-8\n");
      client.println(getJSON());
    }
    else {
      client.println("Content-type: text/html; charset=utf-8\n");
      client.println(welcomeMessage);
      client.println("<h2>Current values</h2>\n<p>");
      String s = "<pre>temperature: "; s+=getTemp(); s+="<br>";
      s +="<pre>humidity: "; s+=getHum(); s+="<br>";
      s +="<pre>brightness-raw: "; s+=getBrightness();
      client.println(s);
      client.println("</p>\n</body>\n</html>");
    }
    
    delay(1);
    client.stop();
  }

  if(ledon && millis()-time>1000) {
    digitalWrite(2, HIGH);
    digitalWrite(16, HIGH);
    ledon = false; time=millis();
  }

  /*if(millis()-serialtime>1000) {
    Serial.println( analogRead(A0) );
    serialtime=millis();
  }*/
  
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

String getJSON() {
  float t = getTemp(); yield();
  digitalWrite(2,HIGH);
  float h = getHum(); yield();
  int b = getBrightness(); yield();
  digitalWrite(2,LOW);
  String s = "{\n";
  s += "  \"temperature\":"; s+=t; s+=",\n";
  s += "  \"humidity\":"; s+=h; s+=",\n";
  s += "  \"brightness-raw\":"; s+=b; s+="\n}";
  return s;
}

/* float getDallasTemp() {
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  byte addr[8];
  float celsius;

  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    ds.reset_search();
    delay(250);
  }

  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  delay(1000);

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  int16_t raw = (data[1] << 8) | data[0];
  byte cfg = (data[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  //// default is 12 bit resolution, 750 ms conversion time

  celsius = (float)raw / 16.0;
  
  Serial.print(", DallasTemp: ");
  Serial.print(celsius);
  return celsius;
} */
