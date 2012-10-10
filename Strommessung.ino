#include "PinConfig.h"

i16 strom;

void stromSetup()
{
}

// misst Strom an Pin mit ACS758
// gibt gemessenen Wert in mA zurück
i16 messeStrom()
{
	// [0:1023] -> [0:3.3]V -> [0:33]A
	// ACS758-50U Daten:
	//   Messbereich:  0 - 50A unidirectional
	//   Sensitivität: 61mV / A (typ. -40°C)
	//   Offset:       0.6V
	//   Offset Drift: +/-40mV bei -40°C
	
	i32 wert = analogRead(pinStrom);
	i32 spannung = wert * 5000 /* mV */ / 1023; // [0:1023] -> [0:3300]mV
	strom    = (spannung - 590 /* mV */) * 1000 /* mA/A */ / (60 /* mV/A */);
	
	// Werte sind durch Datentyp begrenzt!
	return strom;
}
