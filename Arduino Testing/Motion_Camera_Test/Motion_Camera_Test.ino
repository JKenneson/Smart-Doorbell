//Wifi101 library conflicts with Software Serial library (possibly using interrupts on the same pins)
//Need to either figure out how to use Wifi.h library or switch to Mega

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>


// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(69, 3);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

//PIR motion sensor globals
int pirInputPin = 5;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int pirStatus = 0;                    // variable for reading the pin status

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

}

void loop() {
  pirStatus = digitalRead(pirInputPin);  // read input value
  
//  Serial.print("State: ");
//  Serial.println(pirStatus);
  if(pirStatus == HIGH){
    takePicture();
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





