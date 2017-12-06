/**
 * Demo 3 - Senior Design
 * December 2017
 * Jonathan Kenneson & Kyle Monto
 *
 * This demo showcases functionality from the motion detector, camera, and wireless chip.
 * When the motion sensor is triggered, the camera takes a picture and sends it to the Server
 * to be displayed on a webpage.
 */

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <WiFi101.h>
#include <MQTTClient.h>


//*********************************************************************************//
//                             Camera Globals                                      //
//*********************************************************************************//

// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
SoftwareSerial cameraSerial = SoftwareSerial(69, 3);

byte incomingbyte;
int a=0x0000;  //Read Starting address
int j=0;
int k=0;
int count=0;
uint8_t MH,ML;
boolean EndFlag=0;


//*********************************************************************************//
//                           Motion Sensor Globals                                 //
//*********************************************************************************//
int pirInputPin = 5;               // choose the input pin (for PIR sensor)
int pirStatus = 0;                 // variable for reading the pin status


//*********************************************************************************//
//                               Wifi Globals                                      //
//*********************************************************************************//
const int packetSize = 1024;
WiFiClient wclient;
MQTTClient client(packetSize);

char ssid[] = "FBI Van 3";        // your network SSID (name)
char pass[] = "brightquail370";    // your network password (use for WPA, or use as key for WEP)
//char ssid[] = "Embedded Systems Class";        // your network SSID (name)
//char pass[] = "embedded1234";    // your network password (use for WPA, or use as key for WEP)
//char ssid[] = "H2P";        // your network SSID (name)
//char pass[] = "1abc2bc3c4";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;     // the WiFi radio's status


//*********************************************************************************//
//                                  Setup()                                        //
//*********************************************************************************//

void setup() {
  Serial.begin(9600);

  Serial.print(F("Free RAM start: "));
  Serial.println(freeRam());

//  Camera Setup BEGIN
  Serial.print(F("Camera Setup"));
  cameraSerial.begin(38400);

  SendResetCmd();
  delay(2000);          //Delay for bit stabilization
//  ChangeSizeMedium();
  ChangeSizeSmall();
  delay(2000);
  Serial.println(F("... Complete!"));
//  Camera setup END


// Motion Sensor Setup BEGIN
  pinMode(pirInputPin, INPUT);     // Declare sensor as input
  Serial.print(F("Motion Sensor Setup"));
  delay(5000);
  Serial.println(F("... Complete!"));
//  Motion Sensor Setup END


//  Wifi Setup BEGIN
  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(8,7,4);    //WiFi.setPins(chipSelect, irq, reset, enable)  - enable tied to VCC

  client.begin("198.41.30.241", 1883, wclient);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println();
    Serial.println(F("WiFi shield not present. Not continuing execution."));
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
  }

  // Connected now, so print out the data:
  Serial.println(F("Connected to the network!"));
  printWiFiData();
  Serial.println();
  Serial.println(F("Waiting for motion sensor trigger..."));

  //sendMessageToServer("Test");    //Send test message to establish connection
//  Wifi Setup END

  Serial.print(F("Free RAM end setup: "));
  Serial.println(freeRam());
}


//*********************************************************************************//
//                                  main()                                         //
//*********************************************************************************//

void loop() {
  pirStatus = digitalRead(pirInputPin);  // read input value

//  Serial.print(F("State: "));
//  Serial.println(F(pirStatus));
  if(pirStatus == HIGH){
    takePicture();
  }

}




//*********************************************************************************//
//                            Camera Main Functions                                //
//*********************************************************************************//

void takePicture() {
  char pictureArray[packetSize];
  memset(pictureArray, '\0', packetSize);
//  delay(100);

  Serial.print(F("Free RAM taking picture: "));
  Serial.println(freeRam());
  
  SendTakePhotoCmd();

  Serial.println(F("Taking picture"));
  delay(100);

  while(cameraSerial.available()>0) {
    incomingbyte=cameraSerial.read();
  }
  byte b[32];
  int byteCount = 0;

  while(!EndFlag) {
    j=0;
    k=0;
    count=0;
    SendReadDataCmd();

    delay(75); //try going up
    while(cameraSerial.available()>0) {
      incomingbyte=cameraSerial.read();
      k++;
      if((k>5)&&(j<32)&&(!EndFlag)) {
        b[j]=incomingbyte;
        if((b[j-1]==0xFF)&&(b[j]==0xD9))
        EndFlag=1;
        j++;
        count++;
      }
    }

    for(j=0;j<count;j++) {
      if(b[j]==0x00)
      {
//        Serial.print(F("00"));
//        Serial.print(b[j], HEX);
        pictureArray[byteCount] = '0';
        byteCount++;
      }
      else if(b[j]<0x10) {
//        Serial.print(F("0"));
//        Serial.print(b[j], HEX);
        pictureArray[byteCount] = b[j];
        byteCount++;
      }
      else {
//        Serial.print(b[j], HEX);
        pictureArray[byteCount] = b[j];
        byteCount++;
      }

      if(byteCount > packetSize - 32) {
        sendMessageToServer(pictureArray);
        memset(pictureArray, '\0', packetSize);
        byteCount = 0;
      }
    }
  }


  StopTakePhotoCmd(); //stop this picture so another one can be taken
  EndFlag = 0; //reset flag to allow another picture to be read
  
  //Send last data to Server
  sendMessageToServer(pictureArray);
  sendMessageToServer("KFM");

  Serial.println(F("End of picture"));
  Serial.println();

  Serial.print(F("Free RAM end picture: "));
  Serial.println(freeRam());

  delay(5000);
  Serial.println(F("Waiting for motion sensor trigger..."));
}





//*********************************************************************************//
//                           Wifi Main Functions                                   //
//*********************************************************************************//


void sendMessageToServer(char pictureArray []){
// convert into to char array
//  int str_len = stringBuild.length() + 1;  // Length (with one extra character for the null terminator)
//  char char_array[str_len];  // Prepare the character array (the buffer)
//  stringBuild.toCharArray(char_array, str_len);  // Copy it over


//  Serial.println();
//  Serial.print(F("Attempting to publish: "));
//  Serial.println(pictureArray);

  bool published = false;
  // publish data to MQTT broker
  while(!published) {
    if (client.connect("LaunchPadClient")) {
      if(client.publish("KFM", pictureArray, true, 2) == true) {
        Serial.println(F("Publishing successful!"));
        published = true;
      }
      else {
        Serial.println(F("Publishing unsuccessful :("));
//        delay(250);
      }
      client.disconnect();
    }
  }
  
}




//*********************************************************************************//
//                          Camera Helper Functions                                //
//*********************************************************************************//

//Send Reset command
void SendResetCmd() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x26);
  cameraSerial.write((byte)0x00);
}

//Send take picture command
void SendTakePhotoCmd() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x36);
  cameraSerial.write((byte)0x01);
  cameraSerial.write((byte)0x00);

  a = 0x0000; //reset so that another picture can taken
}

void FrameSize() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x34);
  cameraSerial.write((byte)0x01);
  cameraSerial.write((byte)0x00);
}

//Read data
void SendReadDataCmd() {
  MH=a/0x100;
  ML=a%0x100;

  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x32);
  cameraSerial.write((byte)0x0c);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x0a);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)MH);
  cameraSerial.write((byte)ML);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x20);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x0a);

  a+=0x20;
}

void StopTakePhotoCmd() {
  cameraSerial.write((byte)0x56);
  cameraSerial.write((byte)0x00);
  cameraSerial.write((byte)0x36);
  cameraSerial.write((byte)0x01);
  cameraSerial.write((byte)0x03);
}

//160*120
void ChangeSizeSmall() {
    cameraSerial.write((byte)0x56);
    cameraSerial.write((byte)0x00);
    cameraSerial.write((byte)0x31);
    cameraSerial.write((byte)0x05);
    cameraSerial.write((byte)0x04);
    cameraSerial.write((byte)0x01);
    cameraSerial.write((byte)0x00);
    cameraSerial.write((byte)0x19);
    cameraSerial.write((byte)0x22);
}

//320*240
void ChangeSizeMedium()
{
    cameraSerial.write((byte)0x56);
    cameraSerial.write((byte)0x00);
    cameraSerial.write((byte)0x31);
    cameraSerial.write((byte)0x05);
    cameraSerial.write((byte)0x04);
    cameraSerial.write((byte)0x01);
    cameraSerial.write((byte)0x00);
    cameraSerial.write((byte)0x19);
    cameraSerial.write((byte)0x11);
}

//640*480
void ChangeSizeBig()
{
    cameraSerial.write((byte)0x56);
    cameraSerial.write((byte)0x00);
    cameraSerial.write((byte)0x31);
    cameraSerial.write((byte)0x05);
    cameraSerial.write((byte)0x04);
    cameraSerial.write((byte)0x01);
    cameraSerial.write((byte)0x00);
    cameraSerial.write((byte)0x19);
    cameraSerial.write((byte)0x00);
}


//*********************************************************************************//
//                          Wifi Helper Functions                                  //
//*********************************************************************************//

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("\tIP address : "));
  Serial.println(ip);

  Serial.print(F("\tSubnet mask: "));
  Serial.println((IPAddress)WiFi.subnetMask());

  Serial.print(F("\tGateway IP : "));
  Serial.println((IPAddress)WiFi.gatewayIP());

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print(F("\tMAC address: "));
  Serial.print(mac[5], HEX);
  Serial.print(F(":"));
  Serial.print(mac[4], HEX);
  Serial.print(F(":"));
  Serial.print(mac[3], HEX);
  Serial.print(F(":"));
  Serial.print(mac[2], HEX);
  Serial.print(F(":"));
  Serial.print(mac[1], HEX);
  Serial.print(F(":"));
  Serial.println(mac[0], HEX);
  Serial.println();
}






int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
