//Wifi101 library conflicts with Software Serial library (possibly using interrupts on the same pins)
//Need to either figure out how to use Wifi.h library or switch to Mega

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <WiFi101.h>   // changed from Wifi101.h to Wifi.h
#include <PubSubClient.h> 


//*********************************************************************************//
//                             Camera Globals                                      //
//*********************************************************************************//

byte incomingbyte;
//Configure pin 69 and 3 as soft serial port
// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
SoftwareSerial cameraSerial = SoftwareSerial(69, 3); 

int a=0x0000,  //Read Starting address     
    j=0,
    k=0,
    count=0;
uint8_t MH,ML;
boolean EndFlag=0;

String pictureStringAsHex = "";



//*********************************************************************************//
//                           Motion Sensor Globals                                 //
//*********************************************************************************//
int pirInputPin = 5;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int pirStatus = 0;                    // variable for reading the pin status


//*********************************************************************************//
//                               Wifi Globals                                      //
//*********************************************************************************//
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



//*********************************************************************************//
//                                  Setup()                                        //
//*********************************************************************************//

void setup() {
  Serial.begin(9600);

//  Camera Setup BEGIN

  Serial.print("Camera Setup");
  cameraSerial.begin(38400);
  
  SendResetCmd();
  delay(3000);
  ChangeSizeMedium();
  Serial.println("... Complete!");
//  Camera setup END

// Motion Sensor Setup BEGIN

  pinMode(pirInputPin, INPUT);     // declare sensor as input
  Serial.print("Motion Sensor Setup");
  delay(5000);
  Serial.println("... Complete!");
//  Motion Sensor Setup END

//  Wifi Setup BEGIN
  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(8,7,4);    //WiFi.setPins(chipSelect, irq, reset, enable)  - enable tied to VCC

  Serial.println("Wifi Setup");
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

  // Connected now, so print out the data:
  Serial.println("Connected to the network!");
  printWiFiData();
//  Wifi Setup END

  
}


//*********************************************************************************//
//                                  main()                                         //
//*********************************************************************************//

void loop() {
  pirStatus = digitalRead(pirInputPin);  // read input value
  
//  Serial.print("State: ");
//  Serial.println(pirStatus);
  if(pirStatus == HIGH){
    takePicture();
  }

}




//*********************************************************************************//
//                            Camera Main Functions                                //
//*********************************************************************************//

void takePicture() {
  SendTakePhotoCmd();
  
  Serial.println("Taking picture"); 
  delay(100);

  while(cameraSerial.available()>0) {
    incomingbyte=cameraSerial.read();
  }
  byte b[32];
      
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
      if(b[j]<0x10)
      {
        Serial.print("0");
        pictureStringAsHex = pictureStringAsHex + "0";
      }
      Serial.print(b[j], HEX);
      pictureStringAsHex = pictureStringAsHex + String(b[j], HEX);
    }
//    Serial.println();
  }
  Serial.println();
  Serial.println();
  Serial.println(pictureStringAsHex);
  delay(3000);
  StopTakePhotoCmd(); //stop this picture so another one can be taken
  EndFlag = 0; //reset flag to allow another picture to be read
  Serial.println("End of picture");
  Serial.println(); 

  //Send to Server
  sendMessageToServer(pictureStringAsHex);

  
  delay(2000);
}


//*********************************************************************************//
//                           Wifi Main Functions                                   //
//*********************************************************************************//


void sendMessageToServer(String stringBuild){
  // convert into to char array
//  int str_len = stringBuild.length() + 1;  // Length (with one extra character for the null terminator)
  int str_len = 500;
  char char_array[str_len];  // Prepare the character array (the buffer) 
  stringBuild.toCharArray(char_array, str_len);  // Copy it over 

  Serial.println();
  Serial.print("Attempting to publish: ");
  Serial.println(char_array);
  
  // publish data to MQTT broker
  if (client.connect("LaunchPadClient")) {
    if(client.publish("KFM", char_array) == true) {
      Serial.println("Publishing successful!");  
    }
    else {
      Serial.println("Publishing unsuccessful :(");  
    }
    client.disconnect();
  }
  //every 5 seconds
  delay(5000);
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


