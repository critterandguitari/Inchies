#include <OSCBundle.h>
#include <MemoryFree.h>
#include <SLIPEncodedSerial.h>


/*
Make an OSC message and send it over serial
 */

SLIPEncodedSerial SLIPSerial(Serial);

const int ledPin =  3;      // the number of the LED pin

const int rled =  5;      // the number of the LED pin
const int bled =  6;      // the number of the LED pin
const int gled =  7;      // the number of the LED pin

int host_ready = 0;

int msgInSize = 0;

int index = 1; 

String type = "led";
String address = "";


unsigned long previousMillis = 0;        // will store last time knobs sent
const long interval = 100;           // interval at which to send knobs



void LEDcontrol(OSCMessage &msg) {

  int stat;

  digitalWrite(ledPin, LOW);

  // digitalWrite(ledPin, LOW); 
  if (msg.isInt(0)) {
    stat = msg.getInt(0);

    if (stat == 0) {
      digitalWrite(rled, HIGH);
      digitalWrite(gled, HIGH);
      digitalWrite(bled, HIGH);
    }
    if (stat == 1) {
      digitalWrite(rled, HIGH);
      digitalWrite(gled, HIGH);
      digitalWrite(bled, LOW);
    }
    if (stat == 2) {
      digitalWrite(rled, HIGH);
      digitalWrite(gled, LOW);
      digitalWrite(bled, HIGH);
    }
    if (stat == 3) {
      digitalWrite(rled, HIGH);
      digitalWrite(gled, LOW);
      digitalWrite(bled, LOW);
    }
    if (stat == 4) {
      digitalWrite(rled, LOW);
      digitalWrite(gled, HIGH);
      digitalWrite(bled, HIGH);
    }
    if (stat == 5) {
      digitalWrite(rled, LOW);
      digitalWrite(gled, HIGH);
      digitalWrite(bled, LOW);
    }
    if (stat == 6) {
      digitalWrite(rled, LOW);
      digitalWrite(gled, LOW);
      digitalWrite(bled, HIGH);
    }
    if (stat == 7) {
      digitalWrite(rled, LOW);
      digitalWrite(gled, LOW);
      digitalWrite(bled, LOW);
    }
  }
}

// reset to default turn on state
void reset(OSCMessage &msg){
  index = 1;
  host_ready = 0;
    digitalWrite(rled, HIGH);
  digitalWrite(gled, HIGH);
  digitalWrite(bled, HIGH);
    // led solid on, waiting for ready command
  digitalWrite(ledPin, LOW);
}

void renumber(OSCMessage &msg){
  address = "/" + type;
  // send out a 
  OSCMessage  msgOut(address.c_str());
  SLIPSerial.beginPacket();
  msgOut.send(SLIPSerial); 
  SLIPSerial.endPacket(); 
  msgOut.empty();
}

void ready_for_business(OSCMessage &msg) {
  host_ready = 1;
  OSCMessage msgOut("/sys/ready");
  SLIPSerial.beginPacket();  
  msgOut.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  msgOut.empty(); // free space occupied by message
}

void incIndex(OSCMessage &msg){
  index++;
}



void setup() {
  pinMode(ledPin, OUTPUT);      // status LED

  // RGB LED
  pinMode(rled, OUTPUT);
  pinMode(gled, OUTPUT);
  pinMode(bled, OUTPUT);

  digitalWrite(rled, HIGH);
  digitalWrite(gled, HIGH);
  digitalWrite(bled, HIGH);


  int i;
  for(i = 0; i<2; i++){
    digitalWrite(ledPin, HIGH); 
    delay(500);   
    digitalWrite(ledPin, LOW);
    delay(500);

  }

      // led solid on, waiting for ready command
      digitalWrite(ledPin, LOW);



  //begin SLIPSerial just like Serial
  // SLIPSerial.begin(1000000);   // set this as high as you can reliably run on your platform
  SLIPSerial.begin(115200);   // set this as high as you can reliably run on your platform


#if ARDUINO >= 100
  while(!Serial)
    ; //Leonardo "feature"
#endif
}


void loop(){
  // OSCBundle bundleIN;
  OSCMessage msgIn;
  int size;

  while(!SLIPSerial.endofPacket()) {

    if( (size =SLIPSerial.available()) > 0) {
      while(size--) {
        msgIn.fill(SLIPSerial.read());
     //   msgInSize++;
      }
    }
    // disregard messages over 256
   // if (msgInSize > 255) {
   //   msgInSize = 0;
   //   msgIn.empty(); 
   // }
  }
 
  if(!msgIn.hasError()) {
    if (host_ready) {
      digitalWrite(ledPin, LOW);

      // pass it along
      SLIPSerial.beginPacket(); 
      msgIn.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet*/


      // led control
      address = "/" + type +"/" + index + "";
      msgIn.dispatch(address.c_str(), LEDcontrol);

      // renumber
      msgIn.dispatch("/sys/renumber", renumber);

      // reset 
      msgIn.dispatch("/sys/reset", reset);

      // type count
      address = "/" + type;
      msgIn.dispatch(address.c_str(), incIndex);

      msgIn.empty(); // free space occupied by message
      digitalWrite(ledPin, HIGH);

    } // if host is ready for business
    else {
      digitalWrite(ledPin, LOW);// led solid on, waiting for ready command
      msgIn.dispatch("/sys/ready", ready_for_business);
      msgIn.empty(); // free space occupied by message
    }
  }
  else {   // just empty it if there was an error
      msgIn.empty(); // free space occupied by message
  }

}



