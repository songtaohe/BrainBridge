#define MSGTYPE_C2S_FILE_SOURCE			1
#define MSGTYPE_C2S_FILE_INCLUDE		2
#define MSGTYPE_C2S_FILE_LIB			3
#define MSGTYPE_C2S_FILE_FLAG			4
#define MSGTYPE_DEBUG_STRING			111111

#define MSGTYPE_ID_WRAPPER				5



struct MsgBase
{
	int p1;
	int p2;
}; 


struct MsgUniversal
{
	struct MsgBase base;
	int localp;
	char data;
};





