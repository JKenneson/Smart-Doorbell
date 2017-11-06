//Connects ATWINC1500 to wifi network and public MQTT broker, sending a "Hello, world" message every 5 seconds to the nodered server


#include <SPI.h>
#include <WiFi101.h>   // changed from Wifi101.h to Wifi.h
#include <PubSubClient.h>


WiFiClient wclient;
//char ssid[] = "FBI Van 3";        // your network SSID (name)
//char pass[] = "brightquail370";    // your network password (use for WPA, or use as key for WEP)
char ssid[] = "Embedded Systems Class";        // your network SSID (name)
char pass[] = "embedded1234";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;     // the WiFi radio's status

byte server[] = { 198, 41, 30, 241 }; //  Public MQTT Brokers: http://mqtt.org/wiki/doku.php/public_brokers
byte ip[]     = { 172, 16, 0, 100 };

void callback(char* inTopic, byte* payload, unsigned int length){
// Handle callback here

}

PubSubClient client(server, 1883, callback, wclient);



void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);

  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(8,7,4);

  Serial.print("Wifi status: ");

  Serial.println(WiFi.status());
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  printCurrentNet();
  printWiFiData();
}

void loop() {
  Serial.println("Inside loop");

  // convert into to char array
  String stringBuild = "Hello, World";
  int str_len = stringBuild.length() + 1;  // Length (with one extra character for the null terminator)
  char char_array[str_len];  // Prepare the character array (the buffer) 
  stringBuild.toCharArray(char_array, str_len);  // Copy it over 

  
  // publish data to MQTT broker
  if (client.connect("LaunchPadClient")) {
    client.publish("outTopicKMonto", char_array);
//    client.subscribe("inTopicKMonto");
    Serial.println("Publishing successful!");
    client.disconnect();
  }
  //every 5 seconds
  delay(5000);
  

}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP address : ");
  Serial.println(ip);

  Serial.print("Subnet mask: ");
  Serial.println((IPAddress)WiFi.subnetMask());

  Serial.print("Gateway IP : ");
  Serial.println((IPAddress)WiFi.gatewayIP());

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
  Serial.println();
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
  Serial.println();
}


