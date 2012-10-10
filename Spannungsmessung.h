#include "PinConfig.h"

enum BatterieStatus
{
	LEER = 0,
	NORMAL = 1,
	VOLL = 2,
};

void spannungsSetup();
void messeBatterieSpannung();
void pruefeBatterieSpannung();

