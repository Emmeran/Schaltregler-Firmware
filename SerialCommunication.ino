#include "PinConfig.h"

bool headerValid = false;
SerialHeader sHead;

Time sLastPong;

void serialSetup()
{
	Serial.begin(115200);
}

Timer tPing(10 Sekunden);

void sendHeader(SerialCommand cmd, u8 length)
{
    SerialHeader h;
    h.s.magicNumber = SERIAL_MAGIC_NUMBER;
    h.s.commandByte = cmd;
    h.s.packetLength = length;
    Serial.write(h.data, sizeof(SerialHeader));
}

void serialMaintain()
{
    // ping the charger regularily to find out if it's still alive
    if (!headerValid && tPing.check())
	sendHeader(CMDS_PING, 0);
    
	if (!headerValid && Serial.available() >= sizeof(SerialHeader))
	{
	    // skip data until magic number is found
	    while (Serial.available() && Serial.peek() != SERIAL_MAGIC_NUMBER)
		Serial.read();
    
	    if (Serial.available() < sizeof(SerialHeader))
		return;
	    
	    Serial.readBytes((char*)sHead.data, sizeof(SerialHeader));
	    headerValid = true;
			
	    // invalidate header of overlong packets
	    if (sHead.s.packetLength > MAX_SERIAL_PACKET_LENGTH)
		headerValid = false;
	}
	
	// nothing to do?
	if (!headerValid || Serial.available() < sHead.s.packetLength)
		return;
		
	int bytesToRead = sHead.s.packetLength;
		
	switch (sHead.s.commandByte)
	{
	default:
		// invalid command
		break;
	
	case CMDS_BSTATUS: // binary status
	{
	    if (bytesToRead != LADEREGLER_BSTATUS_LENGTH)
		return;
	    
	    u8 i = 0;
	    while (bytesToRead --> 0)
		lrStatus[i++] = Serial.read();
	    
	    break;
	}
	
	case CMDS_STATUS:
	{
	    char buf[bytesToRead + 1];
	    u8 i = 0;
	    
	    while (bytesToRead --> 0)
		buf[i++] = Serial.read();
	    
	    buf[i] = '\n';
	    
/*	    Udp.beginPacket(bcast, UdpPort);
	    udpWriteHeader(CMDU_SYSLOG, i);
	    Udp.write((u8*)buf, i);
	    Udp.endPacket();
*/	    
	    file.write(buf, i + 1);
	}
		break;
		
	case CMDS_PONG:
	{   
	    char c[2];
	    while (bytesToRead --> 0)
	    {
		c[1] = c[0];
		c[0] = Serial.read();
	    }
	    
	    sLastPong = millis();
	    chargerState = c[1] - '0';
	    break;
	}
	}

	// discard remaining data
	while (bytesToRead --> 0)
		Serial.read();

	headerValid = false;			
}


































