#include "PinConfig.h"

void spannungsSetup()
{
}

#define VBAT_RINGBUFFER_SIZE 10
// der Kalibrationsfaktor wird mit der Spannung multipliziert: vbat = [0:1023] * VBAT_CALIBRATION
#define VBAT_CALIBRATION 5 * 443 / 1023

// Batterie Thresholds:
// LEER:
//    Wenn 11.4V unterschritten werden, werden die Ausgänge ausgeschaltet
#define BATTERIE_LEER 		(vbat <  1140 + vbatTC)

// NORMAL: 
//    Wenn 11,7V überschritten werden, werden die Ausgänge wieder angeschaltet
#define BATTERIE_NORMAL 	(vbat >= 1170 + vbatTC && vbat <= 1340 + vbatTC)

// VOLL:
//    Wenn 13,4V überschritten werden, werden die Generatoren abgeschaltet
#define BATTERIE_VOLL 		(vbat  > 1340 + vbatTC)

u32 vbatRingBuffer[VBAT_RINGBUFFER_SIZE];
u8 vbatRingBufferPos = 0;
u32 vbat = 0;
u32 tc = 0;
enum BatterieStatus vbatStatus = LEER;

// misst die Batteriespannung in einem Ringpuffer und berechnet den Mittelwert
void messeBatterieSpannung()
{
	// messe Wert in ADC und speichere ihn in den Ringpuffer
	vbatRingBuffer[vbatRingBufferPos] = analogRead(pinVBat);
	++vbatRingBufferPos %= VBAT_RINGBUFFER_SIZE;

	// berechne den neuen Mittelwert
	int i = 0;
	u32 sum = 0;
	for (; i < VBAT_RINGBUFFER_SIZE; i++)
		sum += vbatRingBuffer[i];
		
	vbat = sum * VBAT_CALIBRATION / VBAT_RINGBUFFER_SIZE;
}

// prüfe Batterie Spannung
void pruefeBatterieSpannung()
{
    messeBatterieSpannung();
    
    if (deviceState == STATE_BOOTUP &&
	vbatRingBufferPos == VBAT_RINGBUFFER_SIZE - 1)
    {
	// taken enough measurements to fill the ring buffer => first reliable result
	deviceState = STATE_POWERSAFE;
	lastStateChange = millis();
    }
	
    if ((deviceState == STATE_POWERSAFE || 
	 deviceState == STATE_OVERPOWER) && BATTERIE_NORMAL)
    {
	deviceState = STATE_NORMAL;
	lastStateChange = millis();
    }
	
    else if ((deviceState == STATE_POWERSAFE ||
	      deviceState == STATE_NORMAL) && BATTERIE_VOLL)
    {
	deviceState = STATE_OVERPOWER;
	lastStateChange = millis();
    }
    
    else if ((deviceState == STATE_NORMAL || 
	      deviceState == STATE_OVERPOWER) && BATTERIE_LEER)
    {
	deviceState = STATE_POWERSAFE;
	lastStateChange = millis();
	
	alleAusgaenge( AUS );
    }
}












