#include <OSCBundle.h>
#include <MemoryFree.h>
#include <SLIPEncodedSerial.h>


/*
Make an OSC message and send it over serial
 */

SLIPEncodedSerial SLIPSerial(Serial);

const int ledPin =  3;      // the number of the LED pin

int host_ready = 0;

int index = 1; 

String type = "key";
String address = "";


unsigned long previousMillis = 0;        // will store last time knobs sent
const long interval = 100;           // interval at which to send knobs

const int key_poll = 1;

int key = 0;
int key_last = 0;

int msgInSize = 0;


// reset to default turn on state
void reset(OSCMessage &msg){
  index = 1;
  host_ready = 0;
  // led solid on, waiting for ready command
  digitalWrite(ledPin, LOW);
}

void ready_for_business(OSCMessage &msg) {
  host_ready = 1;
  OSCMessage msgOut("/sys/ready");
  SLIPSerial.beginPacket();  
  msgOut.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  msgOut.empty(); // free space occupied by message
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

void incIndex(OSCMessage &msg){
  index++;
}



void setup() {
  pinMode(ledPin, OUTPUT);   

  PINC |= 1;  // pullup on key  
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
      //  msgInSize++;
      }
    }
    // disregard messages over 256
   // if (msgInSize > 255) {
  //    msgInSize = 0;
 //     msgIn.empty(); 
 //   }


    // check button, but only if ready
    if (host_ready) {
      unsigned long currentMillis = millis();
      if(currentMillis - previousMillis >= key_poll) {
        previousMillis = currentMillis;   

        key = PINC & 0x1;  //input on PC 0

        if (key != key_last) {
          key_last = key;
          address = "/" + type +"/" + index + "";
          OSCMessage msg(address.c_str());
          msg.add(key);

          SLIPSerial.beginPacket();  
          msg.send(SLIPSerial); // send the bytes to the SLIP stream
          SLIPSerial.endPacket(); // mark the end of the OSC Packet
          msg.empty(); // free space occupied by message

          // toggle led
          pinMode(ledPin, OUTPUT); 
          if (key) digitalWrite(ledPin, HIGH);
          else digitalWrite(ledPin, LOW);

        }
      } /// key poll
    } //  ready  
  }

  if(!msgIn.hasError()) {
    if (host_ready) {

      digitalWrite(ledPin, LOW);

      // pass it along
      SLIPSerial.beginPacket(); 
      msgIn.send(SLIPSerial); // send the bytes to the SLIP stream
      SLIPSerial.endPacket(); // mark the end of the OSC Packet*/

      digitalWrite(ledPin, HIGH);
      
      // renumber
      msgIn.dispatch("/sys/renumber", renumber);

      // reset 
      msgIn.dispatch("/sys/reset", reset);

      // type count
      address = "/" + type;
      msgIn.dispatch(address.c_str(), incIndex);

      msgIn.empty(); // free space occupied by message


    }
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



