#include "PinConfig.h"
#include "OneWire.h"

#define MAX_SENSORS 4

union TemperatureAddress
{
	struct
	{
		u32 a1;
		u32 a2;
	} s;

	u8 data[];
};

extern u8 sensorsFound;

struct TemperatureRole
{
	TemperatureAddress addr;
	char role;
	i16 p[3];
};

union TemperatureMeasurement
{
	struct 
	{
		int16_t celcius10;
		int16_t raw;
		u8 config;
	} s;
	
	u8 data[];
};

extern TemperatureAddress tempAddr[MAX_SENSORS];
extern TemperatureMeasurement tempVals[MAX_SENSORS];
extern TemperatureRole roles[MAX_SENSORS];

extern u8 rolesFound;

enum TempResolution
{
	TEMP_9_BIT = 0,
	TEMP_10_BIT = 1,
	TEMP_11_BIT = 2,
	TEMP_12_BIT = 3
};

void tempSetup();
void tempDiscover();
void logTemperature();
void messeTemperatur();





















