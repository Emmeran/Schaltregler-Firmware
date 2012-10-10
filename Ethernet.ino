#include "PinConfig.h"

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 
  131, 188, 117, 119 };

//EthernetServer server(80);

void ethernetSetup()
{
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
//  server.begin();
}
