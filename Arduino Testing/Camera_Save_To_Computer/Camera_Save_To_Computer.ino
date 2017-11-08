#include <SoftwareSerial.h> 

byte incomingbyte;

//Configure pin 2 and 3 as soft serial port
SoftwareSerial cameraSerial = SoftwareSerial(69, 3); 

int a=0x0000,  //Read Starting address     
    j=0,
    k=0,
    count=0;
uint8_t MH,ML;
boolean EndFlag=0;

String pictureStringAsHex = "";

void setup() { 
  Serial.begin(9600);
  cameraSerial.begin(38400);
  
  SendResetCmd();
  delay(3000);
  ChangeSizeMedium();
  
  delay(3000);
}

void loop() {
  SendTakePhotoCmd();
  
  Serial.println("Start pic"); 
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
  Serial.println("End of pic");
  Serial.println(); 

  delay(2000);
}

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
