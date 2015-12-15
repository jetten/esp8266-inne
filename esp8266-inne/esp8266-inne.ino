#include <ESP8266WiFi.h>
#include <stdlib.h>

#include "DHT.h"        // DHT22 temperature and humidity sensor
#define DHTPIN 13
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

const char welcomeMessage[] PROGMEM = {
  "<!DOCTYPE HTML>\n"
  "<html>\n"
  "<head>\n"
  "  <title>ESP8266 environmental data logger</title>\n"
  "  <link rel=\"stylesheet\" href=\"//jiihon.com/temp/this_server/css/default.css\" type=\"text/css\" />\n"
  "  <link rel=\"shortcut icon\" href=\"//jiihon.com/favicon.ico\" />\n"
  "  <style>\n"
  "    body {font-size: 1.0em;}\n"
  "    pre {display: inline;}\n"
  "    h2 {margin-bottom: 0px;}\n"
  "    p {margin-top: 0px;}\n"
  "  </style>\n"
  "</head>\n"
  "<body>\n"
  "<p>This is the NodeMCU ESP8266 IoT climate sensor.\n"
  "Cached data is available from <a href=\"//inne.jiihon.com\">inne.jiihon.com</a> if you prefer.</p>\n"
  "<p><img src=\"//inne.jiihon.com/nodemcu-sensor.jpg\"/></a>\n"
  "<h2>API access</h2>\n"
  "<p>Access the API from <a href=\"/inne\">/inne</a></p>\n"
  "<h2>API documentation</h2>\n"
  "<div>\n"
  "  The data from the api is in JSON format<br>\n"
  "  <pre>temperature:</pre> Inside temperature in ℃<br>\n"
  "  <pre>humidity:</pre> Humidity in %RH<br>\n"
  "  <pre>brightness-raw:</pre> Raw brightness value, range 0-1023. A value <= 17 usually means that lights are off. Values > 100 means daylight.\n"
  "</div>"
};

//const char* ssid     = "tupsu";
const char* ssid     = "aalto open";

WiFiServer server(80);

unsigned long time = 0;
unsigned long serialtime = 0;
unsigned long submittime = millis();
boolean ledon = true;

unsigned long accquiretime = 0;
unsigned long accquirebrightnesstime = 0;
float temparr[30];
byte temppointer=0;
float humarr[30];
byte humpointer=0;
int brightnessarr[100];
byte brightnesspointer=0;

void setup() {    
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
      client.println("<h2>Current values</h2>\n<div>");
      String s = "<pre>temperature: "; s+=getTemp(); s+=" °C</pre><br>";
      s +="<pre>humidity: "; s+=getHum(); s+=" %RH</pre><br>";
      s +="<pre>brightness-raw: "; s+=getBrightness(); s+="</pre>";
      client.println(s);
      client.println("</div>\n</body>\n</html>");
    }
    
    delay(1);
    client.stop();
  }

  if(ledon && millis()-time>100) {
    digitalWrite(2, HIGH);
    digitalWrite(16, HIGH);
    ledon = false; time=millis();
  }

  /*if(millis()-serialtime>1000) {
    Serial.println( analogRead(A0) );
    serialtime=millis();
  }*/


  if(millis()-accquiretime>1000) {
    temparr[temppointer] = dht.readTemperature();
    humarr[humpointer] = dht.readHumidity();

    if(temppointer>=29) {temppointer=0;} else {temppointer++;}
    if(humpointer>=29) {humpointer=0;} else {humpointer++;}
    accquiretime=millis();
  }
  if(millis()-accquirebrightnesstime>300) {
    brightnessarr[brightnesspointer] = analogRead(A0);
    if(brightnesspointer>=99) {brightnesspointer=0;} else {brightnesspointer++;}
    accquirebrightnesstime=millis();
  }

  if(millis()-submittime>15000 && getBrightness()!=0) {
    digitalWrite(16,LOW);
    ledon=true; time=millis();
    
    submitData();
    submittime=millis();
  }
  
}

// Bubble sort with floats
void isort_float(float *a, int n) {
 for (int i = 1; i < n; ++i) {
   float j = a[i];
   int k;
   for (k = i - 1; (k >= 0) && (j < a[k]); k--) {
     a[k + 1] = a[k];
   }
   a[k + 1] = j;
 }
}

// Bubble sort with floats
void isort(int *a, int n) {
 for (int i = 1; i < n; ++i) {
   int j = a[i];
   int k;
   for (k = i - 1; (k >= 0) && (j < a[k]); k--) {
     a[k + 1] = a[k];
   }
   a[k + 1] = j;
 }
}

float getTemp() {
  return findTemp(temparr);
}
float findTemp(float temperatures[30]) {
  isort_float(temperatures,30);
  return temperatures[15];
}

float getHum() {
  return findHum(humarr);
}
float findHum(float hums[30]) {
  isort_float(hums,30);
  return hums[15];
}

int getBrightness() {
  return findBrightness(brightnessarr);
}
int findBrightness(int brightnesses[100]) {
  isort(brightnesses,100);
  return brightnesses[50];
}

/*int getBrightness() {
  int b[49];
  for(byte i=0; i<49; i++) {
    b[i] = analogRead(A0);
    Serial.print(b[i]); Serial.print(" ");
  }
  //int b[] =  {analogRead(A0), analogRead(A0), analogRead(A0)};
  isort(b, 49);
  return b[24];
}*/

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

void submitData() {

  WiFiClient client;
  if(!client.connect("inne.jiihon.com", 80)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/submit.php?temp=";
  url += getTemp(); yield();
  url += "&hum=";
  url += getHum(); yield();
  url += "&brightness=";
  url += getBrightness(); yield();

  /*Serial.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: inne.jiihon.com\r\n" + 
      "Connection: close\r\n\r\n"); */
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: inne.jiihon.com\r\n" + 
      "Connection: close\r\n\r\n");

  delay(10);

  // Print reply to serial
  /*while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }*/

}




