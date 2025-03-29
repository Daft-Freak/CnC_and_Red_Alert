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


/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer/ WW Library                                *
 *                                                                                             *
 *                    File Name : WINCOMM.H                                                    *
 *                                                                                             *
 *                   Programmer : Steve Tall                                                   *
 *                                                                                             *
 *                   Start Date : 1/10/96                                                      *
 *                                                                                             *
 *                  Last Update : January 10th 1996 [ST]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Overview:                                                                                   *
 *                                                                                             *
 *   These classes was created to replace the greenleaf comms functions used in C&C DOS with   *
 *  WIN32 API calls.                                                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


typedef enum WinCommDialMethodType {
	WC_TOUCH_TONE = 0,
	WC_PULSE
} WinCommDialMethodType;



#define	COMMSUCCESS		0
#define 	ASTIMEOUT      -10
#define	COMMUSERABORT	-16
#define ASSUCCESS COMMSUCCESS
#define ASUSERABORT COMMUSERABORT
typedef void *HANDLE;

// same enum as in the class below
enum
{
    CTS_SET  = 0x10,
    DSR_SET  = 0x20,
    RI_SET   = 0x40,
    CD_SET   = 0x80
};

/*
** WinModemClass.
**
** This class provides access to modems under Win95. The functions are designed to be more or less
** drop in replacements for the Grenleaf comms functions.
*/

class WinModemClass
{

	public:

					WinModemClass (void);		//WinModemClass Contructor
		virtual	~WinModemClass (void);		//WinModemClass Destructor


					/*
					** Serial port open should be called to get a handle to the COM port
					** This needs to be called first as other class members rely on the handle
					**
					** Replacement for Greenleaf function: PortOpenGreenleafFast
					*/
		//virtual	HANDLE	Serial_Port_Open (int port, int baud, int parity, int wordlen, int stopbits);
		virtual	HANDLE	Serial_Port_Open (char *device_name, int baud, int parity, int wordlen, int stopbits, int flowcontrol);

					/*
					** This function releases the COM port handle and should be called after
					** communications have finished
					**
					** Replacement for Greenleaf function: PortClose
					*/
					void		Serial_Port_Close (void);

					/*
					** This member copies any bytes from the internal class serial buffer
					** into your user buffer.
					**
					** Replacement for Greenleaf function: ReadBuffer
					*/
					int		Read_From_Serial_Port (unsigned char *dest_ptr, int buffer_len);

					/*
					** Write chars to the serial port
					**
					** Replacement for Greenleaf function: WriteBuffer
					*/
					void		Write_To_Serial_Port (unsigned char *buffer, int length);

					/*
					** Wait for the outgoing buffer to empty
					*/
					void		Wait_For_Serial_Write (void);

					/*
					** Set the dial type to DIAL_TOUCH_TONE or DIAL_PULSE
					**
					** Replacement for Greenleaf function: HMSetDiallingMethod
					*/
		virtual	void		Set_Modem_Dial_Type (WinCommDialMethodType method);

					/*
					** Get the status of the modem control lines
					** Possible flags are: CTS_SET DSR_SET RI_SET & CD_SET
					**
					** Replacement for Greenleaf function: GetModemStatus
					*/
		virtual	unsigned	Get_Modem_Status (void);

					/*
					** Set the DTR line to the given state
					**
					** Replacement for Greenleaf function: SetDtr
					*/
		virtual	void		Set_Serial_DTR (bool state);

					/*
					** Get the result code from the modem after issuing an 'AT' command
					**
					** Replacement for Greenleaf function: HMInputLine
					*/
		virtual	int		Get_Modem_Result (int delay, char *buffer, int buffer_len);

					/*
					** Issue a dial command to the modem.
					** Use Set_Modem_Dial_Type to select pulse or tone dial
					**
					** Replacement for Greenleaf function: HMDial
					*/
		virtual	void		Dial_Modem (char *dial_number);

					/*
					** Send a command to the modem. This is usually an 'AT' command.
					** Function will optionally retry until 'OK' is received.
					*/
		virtual	int		Send_Command_To_Modem (char *command, char terminator, char *buffer, int buflen, int delay, int retries);

					/*
					** Sets a pointer to a function that will be called for each incoming serial char
					**
					** Replacement for Greenleaf function: HMSetUpEchoRoutine
					*/
		virtual	void		Set_Echo_Function (void(*func)(char c));

					/*
					** Sets a pointer to a function that will be called if ESC is pressed during a dial
					**
					** Replacement for Greenleaf function: HMSetUpAbortKey
					*/
		virtual	void		Set_Abort_Function (int (*func)(void));

					/*
					** Member to allow access to the serial port handle
					*/
					HANDLE	Get_Port_Handle(void);

		/*
		** Modem send result codes
		*/
		enum SendModemEnum {
			MODEM_CMD_TIMEOUT = 0,
			MODEM_CMD_OK,
			MODEM_CMD_0,
			MODEM_CMD_ERROR
		};


		/*
		** Enums for modem status flags
		*/
		enum {
			CTS_SET  = 0x10,
			DSR_SET  = 0x20,
          RI_SET   = 0x40,
          CD_SET   = 0x80
       };

#ifdef PICO_BUILD
	   void RX_Byte(uint8_t b);
#endif

	protected:

		/*
		** Pointer to the internal class circular buffer for incoming data
		*/
		unsigned char				*SerialBuffer;

		/*
		** Head and Tail pointers for our internal serial buffer
		*/
		int							SerialBufferReadPtr;
		int							SerialBufferWritePtr;

		/*
		** Windows handle to the COM port device
		*/
		HANDLE						PortHandle;

		/*
		** Dialing method - DIAL_TOUCH_TONE or DIAL_PULSE
		*/
		WinCommDialMethodType	DialingMethod;
};










/*
** WinNullModemClass.
**
** This class provides access to serial ports under Win95. The functions are designed to be more or less
** drop in replacements for the Grenleaf comms functions.
**
** This class just overloads the WinModemClass members that arent required for direct serial communications
** via a 'null modem' cable.
*/
class WinNullModemClass : public WinModemClass
{

	public:

		virtual	inline	void		Set_Modem_Dial_Type (int){};
		virtual	inline	unsigned	Get_Modem_Status (void){return (0);};
		virtual	inline	void		Set_Serial_DTR (bool){};
		virtual	inline	int		Get_Modem_Result (int, char*, int){return(0);};
		virtual	inline	void		Dial_Modem (char*){};
		virtual	inline	int		Send_Command_To_Modem (char*, char, char*, int, int, int){return (0);};
		virtual	inline	void		Set_Echo_Function (void(*)(char)){};
		virtual	inline	void		Set_Abort_Function (int(*)(void)){};

};


extern WinModemClass *SerialPort;
