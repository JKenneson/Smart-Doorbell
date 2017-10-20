//Wifi101 library conflicts with Software Serial library (possibly using interrupts on the same pins)
//Need to either figure out how to use Wifi.h library or switch to Mega

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <WiFi101.h>   // changed from Wifi101.h to Wifi.h
#include <PubSubClient.h> 

//Camera globals    
//Tx pin = 2
//Rx pin = 3
SoftwareSerial cameraconnection = SoftwareSerial(2, 3);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

//PIR motion sensor globals
int pirInputPin = 5;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int pirStatus = 0;                    // variable for reading the pin status

//Wifi globals
WiFiClient wclient;
char ssid[] = "FBI Van 3";        // your network SSID (name)
char pass[] = "brightquail370";    // your network password (use for WPA, or use as key for WEP)
//char ssid[] = "Embedded Systems Class";        // your network SSID (name)
//char pass[] = "embedded1234";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;     // the WiFi radio's status

byte server[] = { 198, 41, 30, 241 }; //  Public MQTT Brokers: http://mqtt.org/wiki/doku.php/public_brokers
byte ip[]     = { 172, 16, 0, 100 };

void callback(char* inTopic, byte* payload, unsigned int length){
// Handle callback here

}

PubSubClient client(server, 1883, callback, wclient);

void setup() {
  Serial.begin(9600);

//  Camera Setup BEGIN

  Serial.println("KFM Demo 1");
  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

//  Camera setup END

// Motion Sensor Setup BEGIN

  pinMode(pirInputPin, INPUT);     // declare sensor as input
  Serial.println("Motion Sensor Stabilizing...");
  delay(5000);
  Serial.println("Motion Sensor Stabilized");
//  Motion Sensor Setup END

//  Wifi Setup BEGIN
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
//  printCurrentNet();
  printWiFiData();
//  Wifi Setup END

  
}

void loop() {
  pirStatus = digitalRead(pirInputPin);  // read input value
  
  Serial.print("State: ");
  Serial.println(pirStatus);
  if(pirStatus == HIGH){
    takePicture();
    sendMessageToServer("Hello, World");
  }


}

void takePicture(){
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Printing ");
  Serial.print(jpglen, DEC);
  Serial.println(" byte image.");

  int32_t time = millis();
  // Read all the data up to # bytes!
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    String str = (char*) buffer;
    
    Serial.print(str);
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); Serial.println(" ms elapsed");
}

void sendMessageToServer(String stringBuild){
  // convert into to char array
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



