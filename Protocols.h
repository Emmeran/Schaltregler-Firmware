#define SERIAL_MAGIC_NUMBER 0xF7
#define UDP_MAGIC_NUMBER 0xF7

#define MAX_UDP_TX_LENGTH 512

#define LADEREGLER_BSTATUS_LENGTH 24

extern EthernetUDP Udp;

enum SerialCommand
{
	CMDS_PING,
	CMDS_PONG,
	CMDS_REPLY,
	CMDS_STATUS,
	CMDS_LOG,
	CMDS_IOCTL,
	CMDS_TIME,
	CMDS_SILENCE,
	CMDS_BSTATUS,
};

union SerialHeader
{
	struct
	{
		u8 magicNumber;
		u8 commandByte;
		u8 packetLength;
	} s;
	
	u8 data[3];
};

enum UdpCommand
{
	CMDU_PING, // 0
	CMDU_PONG,
	CMDU_REPLY,
	CMDU_STATUS, // 3
	CMDU_LOG,
	CMDU_IOCTL,
	CMDU_TIME, // 6
	CMDU_GETLOG,
	CMDU_LOGPART,
	CMDU_INVOKE, // 9
	CMDU_LREPLY,
	CMDU_SPAMPC,
	CMDU_SPAMREPLY, // 12
	CMDU_STOPSPAM,
	CMDU_GETVALUE,
	CMDU_SENDSTATUS, // 15
	CMDU_SAVEFILE,
	CMDU_GOTPART,
	CMDU_STAYALIVE, // 18
	CMDU_SYSLOG,
	CMDU_RELOAD_TEMPCFG,
	CMDU_SWITCH_GROUP, // 21
	
};

union UdpHeader
{
	struct
	{
		u8 magicNumber;
		u8 commandByte;
		u16 packetLength;
	} s;
	
	u8 data[4];
};


































