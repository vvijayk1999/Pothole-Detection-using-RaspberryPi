#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

String UNAME = "TejasKL";

SoftwareSerial GPS_SoftSerial(4, 5);
TinyGPSPlus gps;

volatile float minutes, seconds;
volatile int degree, secs, mins;

const char* ssid = "Your SSID";                   // wifi ssid
const char* password =  "SSID Password";         // wifi password
const char* mqttServer = "Broker Address";    // IP adress Raspberry Pi
const int mqttPort = 00000;  //Port Number
const char* mqttUser = "Your Username";      // if you don't have MQTT Username, no need input
const char* mqttPassword = "Your Password";  // if you don't have MQTT Password, no need input

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }

  GPS_SoftSerial.begin(9600);
//  client.publish("esp8266", "Hello Raspberry Pi");
//  client.subscribe("esp8266");

}
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (GPS_SoftSerial.available())  /* Encode data read from GPS while data is available on serial port */
      gps.encode(GPS_SoftSerial.read());
    /* Encode basically is used to parse the string received by the GPS and to store it in a buffer so that information can be extracted from it */
  } while (millis() - start < ms);
}
void DegMinSec( double tot_val)   /* Convert data in decimal degrees into degrees minutes seconds form */
{  
  degree = (int)tot_val;
  minutes = tot_val - degree;
  seconds = 60 * minutes;
  minutes = (int)seconds;
  mins = (int)minutes;
  seconds = seconds - minutes;
  seconds = 60 * seconds;
  secs = (int)seconds;
}
  unsigned long start;
        double lat_val, lng_val, alt_m_val;
        uint8_t hr_val, min_val, sec_val;
        bool loc_valid, alt_valid, time_valid;
void callback(char* topic, byte* payload, unsigned int length) {
smartDelay(1000);
        
        lat_val = gps.location.lat(); /* Get latitude data */
        loc_valid = gps.location.isValid(); /* Check if valid location data is available */
        lng_val = gps.location.lng(); /* Get longtitude data */
        alt_m_val = gps.altitude.meters();  /* Get altitude data in meters */
        alt_valid = gps.altitude.isValid(); /* Check if valid altitude data is available */
        hr_val = gps.time.hour(); /* Get hour */
        min_val = gps.time.minute();  /* Get minutes */
        sec_val = gps.time.second();  /* Get seconds */
        time_valid = gps.time.isValid();
        if (!loc_valid)
        {          
          Serial.print("Latitude : ");
          Serial.println("***");
          Serial.print("Longitude : ");
          Serial.println("***");
        }
        else
        {
          DegMinSec(lat_val);
          DegMinSec(lng_val);
        }
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  if (!loc_valid)
        {          
          Serial.print("Latitude : ");
          Serial.println("***");
          Serial.print("Longitude : ");
          Serial.println("***");
        }
        else
        {
          DegMinSec(lat_val);
          DegMinSec(lng_val);
          //latpub.publish(lat_val,6);
          //latpub.publish(lng_val,6);/* Convert the decimal degree value into degrees minutes seconds form */
          //client.publish("Client", "uname,"+String(lat_val)+','+String(lng_val));
          String result = "inslatlong,"+ UNAME + ",0," + String(lat_val,6) + ','+ String(lng_val,6);
          Serial.println(result);
          char xx[50];
          result.toCharArray(xx,50);
          client.publish("Client",xx);
        }
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");

}

void loop() {
  
    client.subscribe("esp8266");
    client.loop();
}
