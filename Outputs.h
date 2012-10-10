#ifndef OUTPUTS_H
#define OUTPUTS_H

namespace Ausgang
{
    // hier die Ausgaenge fuer das entpsrechende Geraet angeben:
    enum Geraet
    {
	Switch,
	WifiAntenne,
	TerminalServer,
	GPS,
	PTUKamera,
	VorschauKamera,
	PanTiltUnit,
	PC,
	HauptKamera,
	PCLueftung,
    };
}

namespace Gruppe
{
    enum Gruppen
    {
	Kommunikation = 1,
	Orientation = 2,
	PreAcquisition = 3,
	Acquisition = 4,
    };
}

#endif
