#include "PinConfig.h"

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))

void error_P(const char* str);
i16 SDReceiveFile(const u8& id, const u16& part, const u16& parts, const char* name);
void SDsendFile(IPAddress ip, u16 port, const char* file, u16 id, u32 bytes);
void SDlog(const char* fmt, ...);
void SDLogStatus();
void SDSetup();






























