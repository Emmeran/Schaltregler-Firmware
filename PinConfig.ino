#include <PinConfig.h>

Timer::Timer( u32 intval )
	: interval(intval), lastCheck(millis())
{
}

bool Timer::check()
{
	if (millis() - lastCheck <= interval)
		return false;

	lastCheck = millis();
	return true;
}

unsigned char ausgang[] = 
{
    A0,
    A1,
    A5,
    A4,
    A3,
    A2,
    
    6,
    5,
    3,
    2,
    9,
    8
};

#define BIN(x) (1 << (sizeof(ausgang) - (x)))

i16 gruppe[] =
{
    // Index = Gruppennummer - 1!
    // Gruppe 1:
    BIN(Ausgang::Switch) | BIN(Ausgang::WifiAntenne),
    
    // Gruppe 2:
    BIN(Ausgang::TerminalServer) | BIN(Ausgang::GPS),
    
    // Gruppe 3:
    BIN(Ausgang::PTUKamera) | BIN(Ausgang::VorschauKamera) | BIN(Ausgang::PanTiltUnit),
    
    // Gruppe 4:
    BIN(Ausgang::PC) | BIN(Ausgang::HauptKamera),
};

void pinSetup()
{
	for (int i = 0; i < sizeof(ausgang); i++)
	{
		pinMode(ausgang[i], OUTPUT);
		digitalWrite(ausgang[i], LOW);
	}
	
	pinMode(pinSDCS, OUTPUT);
	pinMode(pinETHCS, OUTPUT);
	
	digitalWrite(pinSDCS, HIGH);
	digitalWrite(pinETHCS, HIGH);
	
	// setzte ADC auf langsamen Modus (128 prescaler)
	ADCSRA |= 0b00000111;
}

void schalteMOSFET(unsigned int index, unsigned char status)
{
	if (index > sizeof(ausgang))
		return;
		
	digitalWrite(ausgang[index], status);
}

void schalteMOSFETs(i16 indices, unsigned char status)
{
	for (u8 i = 0; i < sizeof(ausgang); i++)
		if (indices & (1 << (sizeof(ausgang) - i)))
			digitalWrite(ausgang[i], status);
}

void schalteGruppe(u8 gruppenIndex, u8 status)
{
    if (gruppenIndex - 1 > sizeof(gruppe))
	return;
    
    schalteMOSFETs(gruppe[gruppenIndex - 1], status);
}

void alleAusgaenge(u8 to)
{
	u8 i;
	// Setzte alle MOSFET Pins auf to
	for (i = 0; i < sizeof(ausgang); i++)
		schalteMOSFET(i, to);
}


