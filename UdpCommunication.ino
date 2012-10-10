#include "PinConfig.h"
#include <Ethernet.h>
#include <EthernetUdp.h>

// maximum size of a packet in the serial protocol, defined mainly by the Serial buffer size

EthernetUDP Udp;
Timer UdpStatus(10000);
u8 chargerState = 0xFF;

#ifdef USE_GPS
Date gpsDate;
EthernetUDP UdpGPS;
#define UdpGPSPort 12010
#endif

IPAddress bcast = IPAddress(255, 255, 255, 255);

char lrStatus[] = { 0xFF };

void udpSetup()
{
	Udp.begin(UdpPort);
	UdpGPS.begin(UdpGPSPort);
}

void udpWriteHeader(UdpCommand cmd, u16 size)
{
	UdpHeader uHeader;
	uHeader.s.magicNumber = UDP_MAGIC_NUMBER;
	uHeader.s.commandByte = cmd;
	uHeader.s.packetLength = size;
	Udp.write(uHeader.data, sizeof(UdpHeader));
}

void udpSendPong(const IPAddress &ip, const u16 &port, const u16 &pingNumber)
{
	Udp.beginPacket(ip, port);
	udpWriteHeader(CMDU_PONG, 2);
	Udp.write((u8*)&pingNumber, 2);
	Udp.endPacket();
}

#ifdef OSZI
Timer x(50);
void sendeSpannung()
{
	if (!x.check())
		return;
	
	Udp.beginPacket(bcast, 8888);
	udpWriteHeader(CMDU_LOG, 2);
	u16 vbat = analogRead(pinVBat);
	Udp.write((u8)(vbat & 0xFF));
	Udp.write((u8)(vbat >> 8));
	i16 strom = analogRead(pinStrom);
	Udp.write((u8)(strom & 0xFF));
	Udp.write((u8)(strom >> 8));
	Udp.endPacket();
}
#endif

void udpPrint(const char* txt)
{
    Udp.beginPacket(bcast, UdpPort);
    udpWriteHeader(CMDU_SYSLOG, strlen(txt));
    Udp.print(txt);
    Udp.endPacket();
}

void udpSendStatus(const IPAddress &ip, const u16 &port)
{
	Udp.beginPacket(ip, port);
	udpWriteHeader(CMDU_STATUS, 1 + sizeof(ausgang) + 4 + 1 + sensorsFound * (8 + sizeof(TemperatureMeasurement)) +1 + rolesFound * 15 + 6 + LADEREGLER_BSTATUS_LENGTH);
	
	Udp.write(sizeof(ausgang));
	for (int i = 0; i < sizeof(ausgang); i++)
		Udp.write(digitalRead(ausgang[i]));
		
	Udp.write((u8)(vbat & 0xFF));
	Udp.write((u8)(vbat >> 8));
	
	Udp.write((u8)(strom & 0xFF));
	Udp.write((u8)(strom >> 8));
	
	Udp.write(sensorsFound);
	
	for (int i = 0; i < sensorsFound; i++)
	{
		Udp.write(tempAddr[i].data, 8);
		Udp.write(tempVals[i].data, sizeof(TemperatureMeasurement));
	}
	
	Udp.write(rolesFound);
	
	for (int i = 0; i < rolesFound; i++)
	{
	    Udp.write(roles[i].addr.data, 8);
	    Udp.write((u8*)&roles[i].role, 1);
	    Udp.write((u8*)&roles[i].p, 6);
	}
	
	u32 tdif = millis() - sLastPong;
	if (tdif > 0xFFFF)
	    tdif = 0xFFFF;
	
	Udp.write((u8)(tdif & 0xFF));
	Udp.write((u8)((tdif >> 8) & 0xFF));
	
	Udp.write((u8)(vbatTC & 0xFF));
	Udp.write((u8)(vbatTC >> 8));
	Udp.write((u8)deviceState);
	Udp.write((u8)chargerState);
	
	Udp.write((u8*)lrStatus, LADEREGLER_BSTATUS_LENGTH);
	
	Udp.endPacket();
}

#ifdef USE_GPS
Time gpsRefreshTime;
#define gpsRefreshInterval 5 Minuten

void udpGPSMaintain()
{
	int packetSize = UdpGPS.parsePacket();
	
	// no packet found
	if (!packetSize)
	    return;
	
	//  $GPRMC,191410,A,4735.5634,N,00739.3538,E,0.0,0.0,181102,0.4,E,A*19
	if (!passed(gpsRefreshTime, gpsRefreshInterval) ||
	    packetSize < sizeof(gpsDate.string) + 6)
	{
	    UdpGPS.flush();
	    return;
	}
	
	u8 buf[] = "$GPRMC,";
	
	for (u8 i = 0; i < sizeof(buf) - 1; i++)
	{
	    if ((u8)UdpGPS.read() != buf[i])
	    {
		UdpGPS.flush();
		return;
	    }
	}
	
	gpsRefreshTime = millis();	
	UdpGPS.read(gpsDate.string, sizeof(gpsDate.string) - 1);
	gpsDate.string[sizeof(gpsDate.string) - 1] = 0;
	
	#define getInt(to, from, size) gpsDate.to = ((gpsDate.string[from] - '0') * 10 + gpsDate.string[from + 1] - '0')
	
	getInt(hour, 0, 2);
	getInt(minute, 2, 2);
	getInt(second, 4, 2);
	getInt(day, 46, 2);
	getInt(month, 48, 2);
	getInt(year, 50, 2);
	gpsDate.acq = millis();
	
	SDlog(PSTR("GPS Time: $GPRMC,%s\n"), gpsDate.string);
	
	UdpGPS.flush();
}
#endif

void udpMaintain()
{
	if (UdpStatus.check())
		udpSendStatus(bcast, UdpPort);
	
#ifdef USE_GPS
	udpGPSMaintain();
#endif

	int packetSize = Udp.parsePacket();
	
	// no packet found
	if (!packetSize)
		return;
		
	IPAddress remoteIP = Udp.remoteIP();
	int remotePort = Udp.remotePort();
	
	if (packetSize < sizeof(UdpHeader))
	{
	    Udp.flush();
	    return;
	}
		
	UdpHeader uHeader;
	Udp.read(uHeader.data, sizeof(UdpHeader));
	
	if (uHeader.s.magicNumber != UDP_MAGIC_NUMBER ||
	    packetSize < sizeof(UdpHeader) + uHeader.s.packetLength)
	{
		Udp.flush();
		return;
	}
		
	switch (uHeader.s.commandByte)
	{
	default:
		//invalid command
		break;
		
	case CMDU_PING:
		if (uHeader.s.packetLength < 2)
		{
			Udp.flush();
			return;
		}
			
		u16 pn;
		Udp.read((u8*)&pn, 2);
		udpSendPong(remoteIP, remotePort, pn);
		break;
		
	case CMDU_SENDSTATUS:
	{
		udpSendStatus(remoteIP, remotePort);
		break;
	}
	
	case CMDU_SAVEFILE:
	{
		if (uHeader.s.packetLength < 5)
			return;
			
		u8 id;
		u16 part, parts, bp = 0;
		int ch;
		char buf[30];
		
		Udp.read(&id, 1);
		Udp.read((u8*)&part, 2);
		Udp.read((u8*)&parts, 2);
		
		while ((ch = Udp.read()) > 0 && bp < sizeof(buf) - 1)
			buf[bp++] = (u8)ch;
			
		buf[bp] = 0;
		i16 bytes = SDReceiveFile(id, part, parts, buf);
		
		Udp.beginPacket(remoteIP, remotePort);
		udpWriteHeader(CMDU_GOTPART, 7);
		
		Udp.write(&id, 1);
		Udp.write((u8*)&part, 2);
		Udp.write((u8*)&parts, 2);
		Udp.write((u8*)&bytes, 2);
		
		Udp.endPacket();
		
		break;
	}
	
	case CMDU_RELOAD_TEMPCFG:
	{
		tempSetup();
		break;
	}
		
	case CMDU_GETLOG:
	{
		if (uHeader.s.packetLength < 8)
		{
			Udp.flush();
			return;
		}

		u16 id;			
		u32 from;
		Udp.read((u8*)&from, 4);
		Udp.read((u8*)&id, 2);
		
		char file[ uHeader.s.packetLength - 5 ];
		Udp.read(file, uHeader.s.packetLength - 6); 
		file[ sizeof(file) - 1 ] = 0;
	
		SDsendFile(remoteIP, remotePort, file, id, from);
		break;
	}
	
	case CMDU_STAYALIVE:
	    lastWatchdogOverride = millis();
	    break;
		
	case CMDU_SWITCH_GROUP:
	{
	    if (uHeader.s.packetLength < 2)
	    {
		Udp.flush();
		return;
	    }
			
	    u8 group = Udp.read();
	    u8 state = Udp.read();
	    
	    schalteGruppe(group, state);
	    
	    break;
	}
	
	case CMDU_IOCTL:
	{
		if (uHeader.s.packetLength < 2)
		{
			Udp.flush();
			return;
		}
			
		u8 channel = Udp.read();
		u8 state = Udp.read();
		
		if (channel > 11)
		{
			Udp.flush();
			return;
		}
			
		Udp.beginPacket(remoteIP, remotePort);
		udpWriteHeader(CMDU_REPLY, 3);
		Udp.write(channel);
		Udp.write(state);

		if (state < 2)
			digitalWrite(ausgang[channel], state == 0 ? LOW : HIGH);
			
		// allow input register to be updated
		__asm__("nop\n\t");
		
		// send new state
		Udp.write(digitalRead(ausgang[channel]));
		Udp.endPacket();
		break;
	}
	}

	Udp.flush();
}






































