#include "PinConfig.h"

void setup() 
{
    wdt_enable(WDTO_4S);
    wdt_reset();
    
    serialSetup();

    Serial.println(FreeRam());

    SDSetup();
    
    pinSetup();
    tempSetup();
  
    stromSetup();
    spannungsSetup();
    
    ethernetSetup();
    udpSetup();
}

DeviceState deviceState = STATE_BOOTUP;

Timer messungen(500 ms);
Timer kommunikation(60 Minuten);
Time lastStateChange;
Time lastWatchdogOverride;

void loop()
{
    wdt_reset();
    
    if (messungen.check())
    {
	pruefeBatterieSpannung();
	messeTemperatur();
    }
	
    switch (deviceState)
    {
    default:
    case STATE_BOOTUP:
	// wait for device to be ready
	break;
    
    // powersafe mode, do nothing except logging to SD
    case STATE_POWERSAFE:
    {
	// regularily communicate to main server
	if (kommunikation.check())
	{
	    deviceState = STATE_POWERSAFE_COMMUNICATING;
	    lastStateChange = millis();
	    lastWatchdogOverride = millis();
	    
	    schalteGruppe(Gruppe::Kommunikation, AN);
	    break;
	}
	
	// log to SD
	SDLogStatus();
	break;
    }
    
    case STATE_POWERSAFE_COMMUNICATING:
    {
	// wait for communcation bootup
	if (!passed(lastStateChange, 30 Sekunden))
	    break;
	
	// take care of UDP communcation and broadcast status
	udpMaintain();
	serialMaintain();
	
	if (!passed(lastWatchdogOverride, 5 Minuten))
	    break;

	// turn off communication & go back to powersafe state
	schalteGruppe(Gruppe::Kommunikation, AUS);
	deviceState = STATE_POWERSAFE;
	lastStateChange = millis();
	break;
    }
    
    case STATE_OVERPOWER:
    case STATE_NORMAL:
    {    
	messeStrom();
	SDLogStatus();

#ifdef OSZI	
	sendeSpannung();
#endif

	serialMaintain();
	udpMaintain();
	break;
    }
    }
}

































