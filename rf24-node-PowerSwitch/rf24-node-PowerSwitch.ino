/*  Remote power switch unit
    will recieve power channel commands
    from the base unit and will report
    change of switch status back to the base
    unit
*/

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Bounce.h>
//#include "printf.h"

// ********** Debug *************
#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINT(x)       Serial.print(x)
  #define DEBUG_PRINTLN(x)     Serial.println(x)
  #define DEBUG_PRINTDEC(x)    Serial.print(x, DEC)
  #define DEBUG_CMD(x)         x
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_CMD(x)
#endif


// ********** I/O Pins *************
#define DEBOUNCE_TIME 5
//                     D4, D3, D2, D5, D6, D7
byte switchInputs[] = {4,  3,  2,  5,  6,  7 };                                     // input switches D2 - D7
byte powerChannel[] = {14, 15, 16, 17, 18, 19};                                	    // power output channels A0 - A5 to SSRs
//                     A0, A1, A2, A3, A4, A5
#define NUM_SWITCHES	sizeof(switchInputs)/sizeof(switchInputs[0])		    // Number of input switches
#define NUM_CHANNELS	sizeof(powerChannel)/sizeof(powerChannel[0])		    // Number of output pins connected to SSRs
Bounce switchDebounce[NUM_SWITCHES] = {Bounce(switchInputs[0], DEBOUNCE_TIME),
                                       Bounce(switchInputs[1], DEBOUNCE_TIME),
                                       Bounce(switchInputs[2], DEBOUNCE_TIME),
                                       Bounce(switchInputs[3], DEBOUNCE_TIME),
                                       Bounce(switchInputs[4], DEBOUNCE_TIME),
                                       Bounce(switchInputs[5], DEBOUNCE_TIME),
                                       };
// http://www.witchmastercreations.com/arduino-simple-midi-footswitch/
// Alternativly could do the following in setup()
// bouncer = (Bounce *) malloc(sizeof(Bounce) * NUM_SWITCHES);                                     
// then a for loop to initialise bouncer[i] = Bounce(switchInputs[i], DEBOUNCE_TIME);
// then bouncer[i].update

																		
// ********** RF24 Variables *************
RF24 radio(9,10);                                                              	// nRF24L01(+) radio attached 
RF24Network network(radio);                                                    	// Network uses that radio
const uint16_t this_node = 1;                                                  	// Address of this node
const uint16_t base_node = 0;                                                 	// Address of the base node
#define C_POWER		   1                                                    // RF24 Message Type - Command
#define S_POWER		   2                                                    // RF24 Message Type - Status

struct payload_t {
  byte channel_status;
  byte channel;
};

void setup(void) { 
  // Serial setup
  DEBUG_CMD(Serial.begin(57600);)
  DEBUG_PRINTLN("******** Debug is On ********");
  DEBUG_PRINTLN("Remote Power Unit");
  
  // I/O setup
  for (byte i = 0; i < NUM_SWITCHES; i++) {
    pinMode(switchInputs[i], INPUT_PULLUP);										// set switch pin to input with internal pullup resistor
  }    
									
  for (byte i = 0; i < NUM_CHANNELS; i++) {
    pinMode(powerChannel[i], OUTPUT);
  }
  
  // RF24 Setup
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
//  DEBUG_CMD(radio.printDetails());
}

void loop(void) {
  network.update();                                                             // Pump the network regularly
  
// ********** RF24 Receiver Input Check ***********************
// Checks for incoming messages header type, then hands
// off to appropriate handler routine
// ************************************************************

  while (network.available()) {  
    RF24NetworkHeader header;
    network.peek(header);
    
    switch (header.type) {                                                      // Dispatch the message to the correct handler
      case C_POWER:
        handle_c_power(header);
        break;
      default:
        DEBUG_PRINTLN("Error - Unknown message header!");
        break;
    }
  }

// ************** Switch Input Check ***********************
// Will check for switch changes and then turn on or off 
// the corresponding power channel will then send channel 
// number and state to the base unit
// *********************************************************

  for(int i = 0; i < NUM_SWITCHES; i++) {
    if(switchDebounce[i].update()) {                                            // cycle through debounce objects to see if switch has changed state
      RF24NetworkHeader header(base_node, S_POWER);				// switch has changed, so prepare the RF24 header
      payload_t message;
      message.channel = i;                                                      // set RE24 message payload to show which switch channel has changed
      
      message.channel_status = switchDebounce[i].read();                        // read the switch state and save to RF24 message payload
      digitalWrite(powerChannel[i], message.channel_status);                    // set the powerChannel output to the state of the input swtich
      network.write(header,&message,sizeof(payload_t));                         // send out our payload
 
      DEBUG_PRINT("Sending RF24 packet for Power Channel: ");
      DEBUG_PRINTDEC(i);
      DEBUG_PRINT(", Switched: ");
      DEBUG_PRINTLN((message.channel_status == HIGH) ? "On" : "Off");
    }
  }
}

// ************* Handle Incoming RF24 Power Command ****************
// Receives a Channel Number (0,1,2,3 etc) and a Status (on or off)
// Channel Number of 255 will act on all channels
// *****************************************************************

void handle_c_power(RF24NetworkHeader& header) {
  payload_t message;
  network.read(header,&message,sizeof(payload_t));
  if (message.channel == 0xFF) {					        // apply channel_status to all lights
    for (int i = 0; i < NUM_CHANNELS; i++) {
      digitalWrite(powerChannel[i], message.channel_status);
    }
  }
  else if (message.channel >= 0 && message.channel < NUM_CHANNELS) {	       // valid Channel Number?
    digitalWrite(powerChannel[message.channel], message.channel_status);       // set receive power channel number to received status
  }
  
  DEBUG_PRINT("Receive RF24 Packet for Power Channel: ");
  DEBUG_PRINTDEC(message.channel);
  DEBUG_PRINT(", Switched: ");
  DEBUG_PRINTLN((message.channel_status == HIGH) ? "On" : "Off");
}
