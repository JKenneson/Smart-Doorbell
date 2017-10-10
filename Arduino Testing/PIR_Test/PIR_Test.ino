/*
 * PIR sensor tester
 * Jonathan Kenneson & Kyle Monto
 */
 
int inputPin = 2;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status
 
void setup() {
  pinMode(inputPin, INPUT);     // declare sensor as input
 
  Serial.begin(9600);
}
 
void loop(){
  val = digitalRead(inputPin);  // read input value

  Serial.print("State: ");
  Serial.println(val);
}
