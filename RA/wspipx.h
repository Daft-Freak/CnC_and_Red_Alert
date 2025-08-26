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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Sun/WSPIPX.h                                               $*
 *                                                                                             *
 *                      $Author:: Joe_b                                                       $*
 *                                                                                             *
 *                     $Modtime:: 8/12/97 5:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WSPIPX_H
#define WSPIPX_H

#include	"wsproto.h"

#ifdef _WIN32
/*
** Include Windows specific extensions for Winsock that allow IPX over winsock 1.1
*/
#include	<wsipx.h>
#else
#if defined(PICO_BUILD) || defined(ESP_BUILD)
#include "picosock.h"
#else
#include <sys/socket.h>
#endif
typedef sockaddr_storage SOCKADDR_IPX;
#endif

/*
** IPX interface class. This handles access to the IPX specific portions of the
** Winsock interface.
**
*/
class IPXInterfaceClass : public WinsockInterfaceClass {

	public:

		IPXInterfaceClass (void);
		//virtual ~IPXInterfaceClass(void){Close();};
		bool Get_Network_Card_Address (int card_number, SOCKADDR_IPX *addr);
#ifndef PORTABLE
	 	virtual long Message_Handler(HWND window, UINT message, UINT wParam, LONG lParam);
#endif
		virtual bool Open_Socket ( SOCKET socketnum );

		virtual ProtocolEnum Get_Protocol (void) {
			return (PROTOCOL_IPX);
		};

		virtual int Protocol_Event_Message (void) {
			return (WM_IPXASYNCEVENT);
		};


	private:
		/*
		** The address of the network we will send broadcasts to. Normally you would expect
		** this to be ff,ff,ff,ff but this fails under NT 4.0. Instead, we can use the network
		** number of the net that this PC is attached to. This limits broadcasts to the current
		** network.
		*/
		unsigned char	BroadcastNet[4];

		/*
		** The node to use as a broadcast address. Normally ff,ff,ff,ff,ff,ff.
		*/
		unsigned char	BroadcastNode[6];

		/*
		** The id of the network cars in this machine.
		*/
		unsigned char	MyNode[6];

		/*
		** The socket number to connect with. Normally this will be virgins reserved socket
		** number - VIRGIN_SOCKET (0x8813).
		*/
		SOCKET			IPXSocketNumber;

};



#endif