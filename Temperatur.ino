#include "PinConfig.h"
#include "OneWire.h"

#define MAX_SENSORS 4


// todo: make copy array so less ram is used
TemperatureAddress tempAddr[MAX_SENSORS];

TemperatureRole roles[MAX_SENSORS];
u8 nRoles = 0;

TemperatureMeasurement tempVals[MAX_SENSORS];

u8 sensorsFound = 0;

Timer tempInterval(5000);
Timer discInterval(60000);
u8 messIndex = 0;
u8 tempWartend = 0;
u8 rolesFound = 0;

i16 vbatTC = 0;

OneWire ds(pinI2C);

#define RESOLUTION(x) ((x >> 5) & 0b00000011)

void tempSetup()
{
	SdFile tempConf;
	tempConf.open(&root, "temp.cfg", O_READ);
	
	char ch;
	char buf[60];
	u8 bp, tap = 0;
	i16 r = 1;
	
	TemperatureRole *tmpArray = roles;//[5];
	
	bool lineStart = true;
	bool validLine = false;
	
	// read file
	while (r > 0)
	{
		bp = 0;
		
		// one line:
		while ((r = tempConf.read(&ch, 1)) == 1)
		{
			if (lineStart)
			{
				validLine = ch == '+';
				lineStart = ch == '\n' ? true : false;
				continue;
			}
	
			if (ch == '\n')
			{
				lineStart = true;
				break;
			}
		
			if (!validLine)
				continue;
			
			if (bp == sizeof(buf))
				continue;
		
			buf[bp++] = ch;
		}
		
		if (bp < 18)
			continue;
			
		#define str2hex(i) (buf[i] < 'A' ? (buf[i] - '0') : (buf[i] + 10 - (buf[i] < 'a' ? 'A' : 'a')))
		
		for (int i = 0; i < 16; i += 2)
			tmpArray[tap].addr.data[i >> 1] = (str2hex(i) << 4) + str2hex(i + 1);
		
		u8 bs = bp;	
		bp = 17;
						
		// todo find role etc:
		u8 bnew = 1;
		#define T_NORMAL   0
		#define T_BINARY   1
		#define T_NEGATIVE 2
		u8 type = T_NORMAL;
		u8 pi = 0;
		i32 val = 0;
		for (; bp <= bs; bp++)
		{
			if (bp == bs || ((buf[bp] < '0' || buf[bp] > '9') && buf[bp] != 'b' && buf[bp] != '-'))
			{
				if (bnew == 0)
				{
					if (pi == 0)
						tmpArray[tap].role = (type == T_NEGATIVE) ? -val : val;
			
					else if (pi < 4)
						tmpArray[tap].p[pi - 1] = (type == T_NEGATIVE) ? -val : val;
						
					pi++;
					type = T_NORMAL;
					val = 0;
				}
			
				bnew = 1;
				continue;
			}
			
			if (bnew && buf[bp] == 'b')
			{
				type = T_BINARY;
				continue;
			}
			
			if (buf[bp] == '-')
			{
				type = T_NEGATIVE;
				continue;
			}
			
			bnew = 0;
			val *= (type == T_BINARY) ? 2 : 10;
			val += buf[bp] - '0';
		}

		if (++tap == sizeof(tmpArray))
			break;
	}
	
	rolesFound = tap;
	tempDiscover();
}

void tempDiscover()
{
	int ptr = 0;
	ds.reset_search();
	
	while (ptr < MAX_SENSORS && ds.search(tempAddr[ptr].data))
	{
#ifdef AnnounceDiscover
		Serial.print("tempDiscover: found ");
		Serial.print(ptr);
		Serial.print(" ");
		Serial.print(tempAddr[ptr].s.a1, HEX);
		Serial.print(":");
		Serial.print(tempAddr[ptr].s.a2, HEX);
		Serial.print("\n");
#endif
		ptr++;
	}
	
	sensorsFound = ptr;
#ifdef AnnounceDiscover
	PgmPrint("tempDiscover: found ");
	Serial.print(ptr);
	PgmPrint(" sensors\n");
#endif
	
	if (messIndex > sensorsFound)
		messIndex = 0;
}

void logTemperature()
{
	SDlog(PSTR(" - Temperatures: (%2d sensors):\n"), sensorsFound);
	for (int i = 0; i < sensorsFound; i++)
		SDlog(PSTR("\t\t%08lX:%08lX: %4dC\n"), tempAddr[i].s.a1, tempAddr[i].s.a2, tempVals[i].s.celcius10);
}

#define HEXOUT(x) if (x < 0x10) Serial.print("0"); Serial.print(x, HEX);

void messeTemperatur()
{
	if (discInterval.check())
		tempDiscover();
		
	if (sensorsFound == 0)
		return;
		
	if (!tempInterval.check())
		return;
	
	if (tempWartend && !ds.done())
		return;
		
	if (tempWartend)
	{
		ds.reset();
		ds.select(tempAddr[messIndex].data);
		ds.write(0xBE); // read scratchpad
		
		byte data[9];
		for (int i = 0; i < 9; i++) 
			data[i] = ds.read();
			
		if (data[8] != OneWire::crc8( data, 8))
		{
			tempVals[messIndex].data[0] = 0xFF;
			tempVals[messIndex].data[1] = 0xFF;
			tempVals[messIndex].data[2] = 0xFF;
			tempVals[messIndex].data[3] = 0xFF;
		}
			
		else
		{
			// valid measurement
			tempVals[messIndex].s.raw = (((int16_t)data[1]) << 8) | data[0];
			tempVals[messIndex].s.config = data[4];
			tempVals[messIndex].s.celcius10 = tempVals[messIndex].s.raw * 5 / 8;
			
			// try to find a role for this measurement
			for (int i = 0; i < rolesFound; i++)
			{
				if (roles[i].addr.s.a1 != tempAddr[messIndex].s.a1 ||
					roles[i].addr.s.a2 != tempAddr[messIndex].s.a2)
					continue;
				
				switch (roles[i].role)
				{
				default:
					break;
					
				case 3:
					if (deviceState == STATE_POWERSAFE ||
					    deviceState == STATE_POWERSAFE_COMMUNICATING)
					    break;
				case 1:
					if (roles[i].p[0] > roles[i].p[1])
					{
						if (tempVals[messIndex].s.celcius10 >= roles[i].p[0])
							schalteMOSFETs(roles[i].p[2], AUS);
						
						else if (tempVals[messIndex].s.celcius10 <= roles[i].p[1])
							schalteMOSFETs(roles[i].p[2], AN);
					}
					else
					{
						if (tempVals[messIndex].s.celcius10 <= roles[i].p[0])
							schalteMOSFETs(roles[i].p[2], AN);
						
						else if (tempVals[messIndex].s.celcius10 >= roles[i].p[1])
							schalteMOSFETs(roles[i].p[2], AUS);
					}
					break;
					
				case 2:
					vbatTC = (tempVals[messIndex].s.celcius10 - 250) * roles[i].p[0] / 100;
					break;
				}
			}
		}
			
		++messIndex %= sensorsFound;
	}
	
	// starte neue Messung
	ds.reset();
	ds.select(tempAddr[messIndex].data);
	ds.write(0x44, 1);
	tempWartend = 1;
}



























