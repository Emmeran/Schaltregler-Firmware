#ifndef PINCONFIG
#define PINCONFIG

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <avr/wdt.h>

#define pinSDCS 4
#define pinETHCS 10
#define pinI2C 7

#define pinVBat 7
#define pinStrom 6

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define i16 int16_t
#define i32 int32_t

#define AN HIGH
#define AUS LOW

#include "Protocols.h"

#include "AtkaSPOT_Schaltregler_v1.h"
#include "SDCard.h"
#include "Ethernets.h"
#include "SerialCommunication.h"
#include "Spannungsmessung.h"
#include "Strommessung.h"
#include "Temperatur.h"
#include "UdpCommunication.h"
#include "State.h"
#include "Outputs.h"

#define ms 
#define Minuten * 60000 ms // 60.000ms = 1min
#define Sekunden * 1000 ms // 1.000ms = 1s

typedef u32 Time;
#define elapsed(since) (millis() - since)
#define passed(since, amount) (elapsed(since) >= amount)

// USE_GPS creates a UDP listener to get the NMEA data from the GPS via the terminal server
#define USE_GPS

#ifdef USE_GPS
struct Date
{
    u8 hour, minute, second;
    u8 day, month, year;
    u8 string[63];
    Time acq;
};

extern Date gpsDate;
#endif

class Timer
{
public:
	Time interval;
	Time lastCheck;
	
	Timer( Time intval );
	bool check();
};

extern Sd2Card card;
extern SdVolume volume;
extern SdFile root;
extern SdFile file;
extern EthernetServer server;
extern u32 vbat;
extern i16 strom;
extern i16 vbatTC;
extern u8 chargerState;

extern Time sLastPong;
extern Time lastStateChange;
extern Time lastWatchdogOverride;

extern unsigned char ausgang[12];

void pinSetup();
#define schalteAusgang(index, status) schalteMOSFET(index, status)
void schalteMOSFET(unsigned int index, unsigned char status);
void schalteMOSFETs(i16 indices, unsigned char status);
void alleAusgaenge(u8 to);
void schalteGruppe(u8 gruppenIndex, u8 status);

#endif

