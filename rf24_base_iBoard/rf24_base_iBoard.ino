//ARDUINO 1.0+ ONLY
//ARDUINO 1.0+ ONLY

#include <digitalWriteFast.h>
#include <iBoardRF24Network.h>
#include <iBoardRF24.h>
#include <Ethernet.h>
#include <SPI.h>
boolean reading = false;

////////////////////////////////////////////////////////////////////////
// RF24
////////////////////////////////////////////////////////////////////////
iBoardRF24 radio(3,8,5,6,7,2);                                                  // nRF24L01(+) radio attached 
iBoardRF24Network network(radio);                                        	// Network uses that radio
const uint16_t this_base_node = 0;                                              // Address of our node
uint16_t remote_node = 1;                                                 	// Address of the other node
#define C_POWER		1                                                       // RF24 Message Type - Command Power On
#define S_POWER		2

struct payload_t {
  byte channel_status;
  byte channel;
};
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// CONFIGURE
////////////////////////////////////////////////////////////////////////
  //byte ip[] = { 192, 168, 0, 177 };   //Manual setup only
  //byte gateway[] = { 192, 168, 0, 1 }; //Manual setup only
  //byte subnet[] = { 255, 255, 255, 0 }; //Manual setup only

  // if need to change the MAC address (Very Rare)
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

  EthernetServer server = EthernetServer(80); //port 80
////////////////////////////////////////////////////////////////////////

void setup(){
  Serial.begin(57600);

  // RF24 Setup
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_base_node);

  // Pins 10,11,12 & 13 are used by the ethernet shield
  
  Ethernet.begin(mac);
  // Ethernet.begin(mac, ip, gateway, subnet); //for manual setup
  //Ethernet.begin(mac, ip);

  server.begin();
  Serial.println(Ethernet.localIP());

}

void loop(){
  // Pump the RF24 network regularly
  network.update();

  // listen for incoming clients, and process qequest.
  checkForClient();
}

void checkForClient(){

  EthernetClient client = server.available();

  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;

    while (client.connected()) {
      if (client.available()) {

        if(!sentHeader){
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          sentHeader = true;
        }

        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info

        if(reading){
          Serial.print(c);

           switch (c) {
            case '2':
              //add code here to trigger on 2
              triggerPin(2, client);
              break;
            case '3':
            //add code here to trigger on 3
              triggerPin(3, client);
              break;
            case '4':
            //add code here to trigger on 4
              triggerPin(4, client);
              break;
            case '5':
            //add code here to trigger on 5
              triggerPin(5, client);
              break;
            case '6':
            //add code here to trigger on 6
              triggerPin(6, client);
              break;
            case '7':
            //add code here to trigger on 7
              triggerPin(7, client);
              break;
            case '8':
            //add code here to trigger on 8
              triggerPin(8, client);
              break;
            case '9':
            //add code here to trigger on 9
              triggerPin(9, client);
              break;
          }

        }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }

      }
    }

    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:

  } 

}

void triggerPin(int pin, EthernetClient client){
//blink a pin - Client needed just for HTML output purposes.  
  
  payload_t message;
  int remote_node = 1;
  message.channel = 2;
  message.channel_status = (pin == 2) ? HIGH : LOW;
  
  RF24NetworkHeader header(remote_node, C_POWER);
  network.write(header,&message,sizeof(payload_t));
  
//  client.print("Turning Lamp ");
  client.println((message.channel_status == HIGH) ? "on" : "off");
//  client.print(" Channel ");
//  client.println(pin);
//  client.print("<br>");
  

//  digitalWrite(pin, HIGH);
//  delay(25);
//  digitalWrite(pin, LOW);
//  delay(25);
}
