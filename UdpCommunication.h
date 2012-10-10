#include "PinConfig.h"
#include <Ethernet.h>
#include <EthernetUdp.h>

#define UdpPort 8888

extern IPAddress bcast;
extern EthernetUDP Udp;

extern char lrStatus[23];

void udpSetup();
void udpWriteHeader(UdpCommand cmd, u16 size);
void udpSendPong(const IPAddress &ip, const u16 &port, const u16 &pingNumber);
void sendeSpannung();
void udpSendStatus(const IPAddress &ip, const u16 &port);
void udpMaintain();
void udpPrint(const char* txt);




































