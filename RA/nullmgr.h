/*
**	Command & Conquer Red Alert(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Header: /CounterStrike/NULLMGR.H 1     3/03/97 10:25a Joe_bostic $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : CONNECT.H                                *
 *                                                                         *
 *                   Programmer : Bill Randolph                            *
 *                                                                         *
 *                   Start Date : December 19, 1994                        *
 *                                                                         *
 *                  Last Update : April 3, 1995   [BR]                 		*
 *                                                                         *
 *-------------------------------------------------------------------------*
 *                                                                         *
 * This is the Connection Manager for a NULL-Modem connection.					*
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef NULLMODEM_H
#define NULLMODEM_H


/*
********************************* Includes **********************************
*/
#include "nullconn.h"
#include "connmgr.h"
#ifndef WIN32
#include "commlib.h"
#endif

/*
***************************** Class Declaration *****************************
*/
class NullModemClass : public ConnManClass
{
	/*
	---------------------------- Public Interface ----------------------------
	*/
	public:
		enum SendModemEnum {
			MODEM_CMD_TIMEOUT = 0,
			MODEM_CMD_OK,
			MODEM_CMD_0,
			MODEM_CMD_ERROR
		};

		char *BuildBuf;
		int MaxLen;

		char *EchoBuf;
		int EchoSize;
		int EchoCount;

		int OldIRQPri;

		int ModemVerboseOn;
		int ModemEchoOn;
		int ModemWaitCarrier;
		int ModemCarrierDetect;
		int ModemCarrierLoss;
		int ModemHangupDelay;
		int ModemGuardTime;
		char ModemEscapeCode;

		static void (*OrigAbortModemFunc)(int);
		static KeyNumType Input;
		static GadgetClass *Commands;							// button list

		/*
		**	Constructor/destructor.
		*/
		NullModemClass (int numsend, int numreceive, int maxlen, unsigned short magicnum);
		virtual ~NullModemClass ();

		/*
		**	This is the main initialization routine.
		*/
		int Init( int port, int irq, char *dev_name, int baud, char parity, int wordlength, int stopbits, int flowcontrol );
		int Delete_Connection( void );
		virtual int Num_Connections(void);
		virtual int Connection_ID(int ) {return (0);}
		virtual int Connection_Index(int ) {return (0);}
		int Init_Send_Queue( void );
		void Shutdown( void );

		virtual void Set_Timing (unsigned long retrydelta,
			unsigned long maxretries, unsigned long timeout);

		/*
		**	This is how the application sends & receives messages.
		*/
		int Send_Message (void *buf, int buflen, int ack_req = 1);
		int Get_Message (void *buf, int *buflen);

		/*
		** These are for compatibility
		*/
		virtual int Send_Private_Message (void *buf, int buflen,
			int ack_req = 1, int = CONNECTION_NONE)
			{return (Send_Message(buf,buflen,ack_req));}
		virtual int Get_Private_Message (void *buf, int *buflen, int *)
			{return (Get_Message(buf,buflen));}

		/*
		**	The main polling routine; should be called as often as possible.
		*/
		virtual int Service (void);

		/*
		**	Queue utility routines.  The application can determine how many
		**	messages are in the send/receive queues, and the queue's average
		**	response time (in clock ticks).
		*/
		int Num_Send(void);
		int Num_Receive(void);
		virtual unsigned long Response_Time(void);
		virtual void Reset_Response_Time(void);
		void * Oldest_Send(void);
		virtual void Configure_Debug(int index, int type_offset, int type_size,
			char **names, int namestart, int namecount);
#ifdef CHEAT_KEYS
		virtual void Mono_Debug_Print(int index, int refresh = 0);
#endif

		/*
		** These are for compatibility
		*/
		virtual int Global_Num_Send(void) {return (Num_Send());}
		virtual int Global_Num_Receive(void) {return (Num_Receive());}
		virtual int Private_Num_Send(int = CONNECTION_NONE)
			{return (Num_Send());}
		virtual int Private_Num_Receive(int = CONNECTION_NONE)
			{return (Num_Receive());}

		DetectPortType Detect_Port( SerialSettingsType *settings );
		int Detect_Modem( SerialSettingsType *settings, bool reconnect = 0 );
		DialStatusType Dial_Modem(char *string, DialMethodType method, bool reconnect = 0);
		DialStatusType Answer_Modem(bool reconnect = 0);
		bool Hangup_Modem(void);
		void Setup_Modem_Echo(void (*func)(char c));
		void Remove_Modem_Echo(void);
		void Print_EchoBuf(void);
		void Reset_EchoBuf(void);
#ifdef WIN32
		static int Abort_Modem();
#else
		static int Abort_Modem(PORT *);
#endif
		void Setup_Abort_Modem(void);
		void Remove_Abort_Modem(void);

		int Change_IRQ_Priority(int irq);
		int Get_Modem_Status(void);
		int Send_Modem_Command( char *command, char terminator, char *buffer, int buflen, int delay, int retries );
		int Verify_And_Convert_To_Int( char *buffer );

	/*
	**	Private Interface.
	*/
	private:

		/*
		**	This is a pointer to the NULL-Modem Connection object.
		*/
		NullModemConnClass *Connection;
		int NumConnections;						// # connection objects in use

#ifdef WIN32
		/*
		** This is the Win95 port handle
		*/
		HANDLE	PortHandle;
#else	//WIN32
		/*
		**	This is the Greenleaf port handle.
		*/
		PORT *Port;
#endif	//WIN32

		int NumSend;
		int NumReceive;
		unsigned short MagicNum;

		/*
		**	This is the staging buffer for parsing incoming packets.
		**	RXSize is the allocated size of the RX buffer.
		**	RXCount is the # of characters we currently have in our buffer.
		*/
		char *RXBuf;
		int RXSize;
		int RXCount;

		/*.....................................................................
		Timing parameters for all connections
		.....................................................................*/
		unsigned long RetryDelta;
		unsigned long MaxRetries;
		unsigned long Timeout;

		/*
		**	Various Statistics
		*/
		int SendOverflows;
		int ReceiveOverflows;
		int CRCErrors;
};

#endif

/*************************** end of nullmgr.h ******************************/
