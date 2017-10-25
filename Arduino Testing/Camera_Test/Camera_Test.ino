//Tx pin = 52 must be digital
//Rx pin = 50 must be analog

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>   
#include <Base64.h>      

// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(69, 3);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

void setup() {
  Serial.begin(9600);
  Serial.println("VC0706 Camera snapshot test");
  
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
  
//  cam.setImageSize(VC0706_640x480);        // biggest
//  cam.setImageSize(VC0706_320x240);        // medium
  cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

  Serial.println("Snap in 3 secs...");
  delay(3000);

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
    char imageBuffer[72];
    uint8_t * temp;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    temp =  cam.readPicture(bytesToRead);
    strncpy(imageBuffer, temp, bytesToRead); 
//    String str = (char*) buffer;
//    Serial.print(imageBuffer); Serial.print(" = ");
//    Todo: Print in binary and convert output to jpeg
    int encodedLen = base64_enc_len(bytesToRead);
    char encoded[encodedLen + 1];
    // note input is consumed in this step: it will be empty afterwards
    base64_encode(encoded, imageBuffer, bytesToRead); 

    for(int i = 0; i < bytesToRead; i++){
      Serial.print(encoded[i]);
    }
//    Serial.print(encoded);
//    Serial.print(str);
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); Serial.println(" ms elapsed");
}

void loop() {
}

