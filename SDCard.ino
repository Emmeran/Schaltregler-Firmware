#include "PinConfig.h"

/************ SDCARD STUFF ************/
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))

void error_P(const char* str) {
  SerialPrintln_P(str);
  if (card.errorCode()) {
    PgmPrint("SD! ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }
  while(1);
}

i16 SDReceiveFile(const u8& id, const u16& part, const u16& parts, const char* name)
{
	static u8 currentID = 0;
	static u16 expectedPart = 0;
	
	char buf[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	if (currentID != 0)
	{
		snprintf(buf, 12, "temp%03d.dat", currentID);
	
		if (currentID != id && part == 0)
			SdFile::remove(&root, buf);
			
		expectedPart = 0;
	}
	
	// ignore invalid parts
	if (part != expectedPart)
		return 0;
	
	snprintf(buf, 12, "temp%03d.dat", id);
	SdFile in;
	in.open(&root, buf, expectedPart == 0 ? O_RDWR | O_CREAT | O_TRUNC | O_SYNC : O_RDWR | O_AT_END | O_SYNC);
	
	i16 bytes;
	u16 wbytes = 0;
	while ((bytes = Udp.read(buf, 64)) > 0)
		wbytes += in.write(buf, bytes);
		
	expectedPart = part + 1;
	
	if (part == parts)
	{
		currentID = 0;
		expectedPart = 0;
		
		SdFile::remove(&root, name);
		in.rename(&root, name);
	}

	in.close();
	return wbytes;
}

void SDsendFile(IPAddress ip, u16 port, const char* file, u16 id, u32 bytes)
{
	SdFile fo;
	if (!fo.open(&root, file, O_READ))
	{
		Udp.beginPacket(ip, port);
		udpWriteHeader(CMDU_LOG,  6);
		
		Udp.write((u8*)&id, 2);
		Udp.write((u8)0);
		Udp.write((u8)0);
		Udp.write((u8)0);
		Udp.write((u8)0);
		Udp.endPacket();
		return;
	}
	
	u32 s = fo.fileSize();
	
	if (s < bytes)
		bytes = s;
	
	if (bytes != 0)
		fo.seekEnd(-bytes);
		
	else
		bytes = s;
	
	i16 read = 0;
	u16 written = 0;
	u16 part = 0;
	u16 parts = (bytes - 1) / MAX_UDP_TX_LENGTH + 1;
	while (bytes > 0)
	{
	    wdt_reset();
	    
		Udp.beginPacket(ip, port);
		udpWriteHeader(CMDU_LOG, (bytes > MAX_UDP_TX_LENGTH ? MAX_UDP_TX_LENGTH : bytes) + 6);
		
		Udp.write((u8*)&id, 2);
		Udp.write((u8*)&part, 2);
		Udp.write((u8*)&parts, 2);
		
		part++;
		
		u8 buf[8];
		written = 0;
		while (bytes > 0 && written < MAX_UDP_TX_LENGTH)
		{
			read = fo.read(buf, bytes > sizeof(buf) ? sizeof(buf) : bytes);
			
			if (read != (bytes > sizeof(buf) ? sizeof(buf) : bytes))
				bytes = 0;
				
			else
				bytes -= read;
			
			written += read;
			Udp.write(buf, read);
		}
		
		Udp.endPacket();
	}
}

void SDlog(const char* fmt, ...)
{
	char buf[80];	
	va_list args;
	va_start(args, fmt);
	int bytes = vsnprintf_P(buf, sizeof(buf), fmt, args);
	file.write(buf, bytes > sizeof(buf) ? sizeof(buf) : bytes);
//	Serial.write((uint8_t*)buf, bytes > 80 ? 80 : bytes);
	va_end(args);
}

Timer SDLogTimer(30000);
void SDLogStatus()
{
	if (!SDLogTimer.check())
		return;

	SDlog(PSTR("%10lu: STATUS Vbat(%4u V/10) Cur(%5d mA) LRP(%6d) - Outputs: "), millis(), vbat, strom, millis() - sLastPong);
	
	for (int i = 0; i < sizeof(ausgang); i++)
		SDlog(i % 5 == 4 ? PSTR("%1d ") : PSTR("%1d"), digitalRead(ausgang[i]));

	logTemperature();		
	
	SDlog(PSTR("\n"));
}

void SDSetup()
{
	if (!card.init(SPI_HALF_SPEED, pinSDCS) ||
		!volume.init(&card) ||
		!root.openRoot(&volume)) 
		error("SD Init failed");

	if (!file.open(&root, "system.log", O_RDWR | O_CREAT | O_AT_END | O_SYNC )) error("cannot open logfile");
}































