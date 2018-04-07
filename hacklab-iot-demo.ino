
/***************************************************
  Adafruit MQTT Library ESP8266 Example
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino
  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>


#define DHTPIN 12
#define DHTTYPE DHT11
/************************* WiFi Access Point *********************************/

//#define WLAN_SSID       "Chairman."
//#define WLAN_PASS       "gob31119"
#define WLAN_SSID "Lumia 650 2271"
#define WLAN_PASS "save2000"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "m11.cloudmqtt.com"
//#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_SERVERPORT  12063                   // use 8883 for SSL
#define AIO_USERNAME    "miidvfuj"
#define AIO_KEY         "emujcuPw9MFs"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/sensor/temp");
Adafruit_MQTT_Publish hum = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/sensor/hum");
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/

DHT climate(DHTPIN, DHTTYPE);

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
uint32_t humidity = 0,
         temperature = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  climate.begin();
  Serial.println(F("Hacklab IOT demo"));

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

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

uint32_t x = 0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }

  humidity = climate.readHumidity();
  temperature = climate.readTemperature();
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("\nCannot read temperature and humdity, please contact support.");
    return;
  }
  String payload = "{ \"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
  // Now we can publish stuff!

  if (! (temp.publish(temperature) && hum.publish(humidity) )) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  //  mqtt.publish("/sensor", payload);
  //  ping the server to keep the mqtt connection alive
  //  NOT required if you are publishing once every KEEPALIVE seconds

  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
  delay(1000);
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
