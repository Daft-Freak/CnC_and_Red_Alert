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
 *                     $Archive:: /Sun/WSPUDP.cpp                                             $*
 *                                                                                             *
 *                      $Author:: Joe_b                                                       $*
 *                                                                                             *
 *                     $Modtime:: 8/05/97 6:45p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *  WSProto.CPP WinsockInterfaceClass to provide an interface to Winsock protocols             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * UDPInterfaceClass::UDPInterfaceClass -- Class constructor.                                  *
 * UDPInterfaceClass::Set_Broadcast_Address -- Sets the address to send broadcast packets to   *
 * UDPInterfaceClass::Open_Socket -- Opens a socket for communications via the UDP protocol    *
 * TMC::Message_Handler -- Message handler function for Winsock related messages               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"
#include	"internet.h"
#include	"wspudp.h"

#include	<assert.h>
#include	<stdio.h>

#ifdef _WIN32
#include	<svcguid.h>
#include	<nspapi.h>

typedef int socklen_t;
#else
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef PICO_BUILD
#define gethostname(name, size) name[0] = 0;
#endif

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define OutputDebugString(x) printf("%s", x)
#endif

/***********************************************************************************************
 * UDPInterfaceClass::UDPInterfaceClass -- Class constructor.                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/5/97 12:11PM ST : Created                                                              *
 *=============================================================================================*/
UDPInterfaceClass::UDPInterfaceClass (void) : WinsockInterfaceClass()
{}



/***********************************************************************************************
 * UDPIC::~UDPInterfaceClass -- UDPInterface class destructor                                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    10/9/97 12:17PM ST : Created                                                             *
 *=============================================================================================*/
UDPInterfaceClass::~UDPInterfaceClass (void)
{
	while ( BroadcastAddresses.Count() ) {
		delete BroadcastAddresses[0];
		BroadcastAddresses.Delete(0);
	}

	while ( LocalAddresses.Count() ) {
		delete LocalAddresses[0];
		LocalAddresses.Delete(0);
	}

	Close();
}


/***********************************************************************************************
 * UDPInterfaceClass::Set_Broadcast_Address -- Sets the address to send broadcast packets to   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to address in decimal dot format. i.e. xxx.xxx.xxx.xxx                        *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/5/97 12:12PM ST : Created                                                              *
 *=============================================================================================*/
void UDPInterfaceClass::Set_Broadcast_Address (void *address)
{
	char* ip_addr = (char*) address;
	assert ( strlen (ip_addr) <= strlen ( "xxx.xxx.xxx.xxx" ) );

	unsigned char *baddr = new unsigned char[4];

	uint32_t addr = inet_addr(ip_addr);
	memcpy(baddr, &addr, 4);
	BroadcastAddresses.Add (baddr);
}



/***********************************************************************************************
 * UDPInterfaceClass::Open_Socket -- Opens a socket for communications via the UDP protocol    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Socket number to use. Not required for this protocol.                             *
 *                                                                                             *
 * OUTPUT:   True if socket was opened OK                                                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/5/97 12:13PM ST : Created                                                              *
 *=============================================================================================*/
bool UDPInterfaceClass::Open_Socket ( SOCKET )
{
	linger ling;
	struct 	sockaddr_in addr;

	/*
	** If Winsock is not initialised then do it now.
	*/
	if ( !WinsockInitialised ) {
		if ( !Init()) return ( false );;
	}

	/*
	** Create our UDP socket
	*/
	Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (Socket == INVALID_SOCKET) {
		return (false);
	}

	/*
	** Bind our UDP socket to our UDP port number
	*/
	addr.sin_family = AF_INET;
	addr.sin_port = (unsigned short) htons ( (unsigned short) PlanetWestwoodPortNumber);
	addr.sin_addr.s_addr = htonl (INADDR_ANY);

	if ( bind (Socket, (sockaddr *)&addr, sizeof(addr) ) == SOCKET_ERROR) {
		Close_Socket ();
		return (false);
	}

	/*
	** Use gethostbyname to find the name of the local host. We will need this to look up
	** the local ip address.
	*/
	char hostname[128];
	gethostname(hostname, 128);
	WWDebugString (hostname);
	struct hostent *host_info = gethostbyname ( hostname );

	/*
	** Clear out any old local addresses from the local address list.
	*/
	while ( LocalAddresses.Count() ) {
		delete LocalAddresses[0];
		LocalAddresses.Delete(0);
	}


	/*
	** Add all local IP addresses to the list. This list will be used to discard any packets that
	** we send to ourselves.
	*/
	unsigned long **addresses = (unsigned long**) (host_info->h_addr_list);

	for ( ;; ) {
		if ( !*addresses ) break;

		unsigned long address = **addresses++;
		//address = ntohl (address);

		char temp[128];
		sprintf (temp, "RA95: Found local address: %d.%d.%d.%d\n", address & 0xff, (address & 0xff00) >> 8, (address & 0xff0000) >> 16, (address & 0xff000000) >> 24);
		OutputDebugString (temp);

		unsigned char *a = new unsigned char [4];
		* ((unsigned long*) a) = address;
		LocalAddresses.Add (a);
	}

	/*
	** Set options for the UDP socket
	*/
	ling.l_onoff = 0;		// linger off
	ling.l_linger = 0;	// timeout in seconds (ie close now)
	setsockopt (Socket, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));

	// enable broadcast
	int yes = 1;
	setsockopt (Socket, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(int));

	WinsockInterfaceClass::Set_Socket_Options();

	return (true);


}




/***********************************************************************************************
 * UDPIC::Broadcast -- Send data via the Winsock socket                                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to buffer containing data to send                                             *
 *           length of data to send                                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/20/96 3:00PM ST : Created                                                              *
 *=============================================================================================*/
void UDPInterfaceClass::Broadcast (void *buffer, int buffer_len)
{
	for ( int i=0 ; i<BroadcastAddresses.Count() ; i++ ) {

		/*
		** Create a temporary holding area for the packet.
		*/
		WinsockBufferType *packet = new WinsockBufferType;

		/*
		** Copy the packet into the holding buffer.
		*/
		memcpy ( packet->Buffer, buffer, buffer_len );
		packet->BufferLen = buffer_len;

		/*
		** Indicate that this packet should be broadcast.
		*/
		packet->IsBroadcast = true;

		/*
		** Set up the send address for this packet.
		*/
		memset (packet->Address, 0, sizeof (packet->Address));
		memcpy (packet->Address+4, BroadcastAddresses[i], 4);

		/*
		** Add it to our out list.
		*/
		OutBuffers.Add ( packet );

#ifdef PORTABLE
		// enable write events
		Socket_Check_Write(Socket, true);
#else
		/*
		** Send a message to ourselves so that we can initiate a write if Winsock is idle.
		*/
		SendMessage ( MainWindow, Protocol_Event_Message(), 0, (LONG)FD_WRITE );
#endif

		/*
		** Make sure the message loop gets called.
		*/
		Keyboard->Check();
	}
}




#ifdef PORTABLE
// like below, but less windows-y
void UDPInterfaceClass::Event_Handler(int socket, SocketEvent event)
{
	struct sockaddr_in addr;
	WinsockBufferType *packet;

	switch(event)
	{
		case SOCKEV_READ:
		{
			/*
			** Call the recvfrom function to get the outstanding packet.
			*/
			socklen_t addr_len = sizeof(addr);
			int rc = recvfrom ( Socket, (char*)ReceiveBuffer, sizeof (ReceiveBuffer), 0, (sockaddr *)&addr, &addr_len);
			if (rc == SOCKET_ERROR) {
				Clear_Socket_Error (Socket);
				return;
			}

			/*
			** rc is the number of bytes received
			*/
			if ( rc ) {

				/*
				** Make sure this packet didn't come from us. If it did then throw it away.
				*/
				for ( int i=0 ; i<LocalAddresses.Count() ; i++ ) {
					if ( ! memcmp (LocalAddresses[i], &addr.sin_addr.s_addr, 4) ) return;
				}

				/*
				** Create a new buffer and store this packet in it.
				*/
				packet = new WinsockBufferType;
				packet->BufferLen = rc;
				memcpy ( packet->Buffer, ReceiveBuffer, rc );
				memset ( packet->Address, 0, sizeof (packet->Address) );
				memcpy ( packet->Address+4, &addr.sin_addr.s_addr, 4 );
				InBuffers.Add (packet);
			}
			break;
		}
		case SOCKEV_WRITE:
		{
			/*
			** If there are no packets waiting to be sent then bail.
			*/
			if ( OutBuffers.Count() == 0 ) {
				Socket_Check_Write(Socket, false); // don't need to be notified any more
				return;
			}
			int packetnum = 0;

			/*
			** Get a pointer to the packet.
			*/
			packet = OutBuffers [ packetnum ];

			/*
			** Set up the address structure of the outgoing packet
			*/
			addr.sin_family = AF_INET;
			addr.sin_port = (unsigned short) htons ((unsigned short)PlanetWestwoodPortNumber);
			memcpy (&addr.sin_addr.s_addr, packet->Address+4, 4);

			/*
			** Send it.
			** If we get a WSAWOULDBLOCK error it means that Winsock is unable to accept the packet
			** at this time. In this case, we clear the socket error and just exit. Winsock will
			** send us another WRITE message when it is ready to receive more data.
			*/
			int rc = sendto ( Socket, (const char*) packet->Buffer, packet->BufferLen, 0, (sockaddr *)&addr, sizeof (addr) );

			if (rc == -1){
				if (Get_Last_Error() == EWOULDBLOCK) {
					Clear_Socket_Error (Socket);
					return;
				}
			}

			/*
			** Delete the sent packet.
			*/
			OutBuffers.Delete ( packetnum );
			delete packet;

			break;
		}
	}
}
#else
/***********************************************************************************************
 * TMC::Message_Handler -- Message handler function for Winsock related messages               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Windows message handler stuff                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/20/96 3:05PM ST : Created                                                              *
 *=============================================================================================*/

long UDPInterfaceClass::Message_Handler(HWND, UINT message, UINT, LONG lParam)
{
	struct 	sockaddr_in addr;
	int	 	rc;
	int		addr_len;
	WinsockBufferType *packet;

	/*
	** We only handle UDP events.
	*/
	if ( message != WM_UDPASYNCEVENT ) return (1);

	/*
	** Handle UDP packet events
	*/
	switch ( WSAGETSELECTEVENT(lParam) ) {

		/*
		** Read event. Winsock has data it would like to give us.
		*/
		case FD_READ:
			/*
			** Clear any outstanding errors on the socket.
			*/
			rc = WSAGETSELECTERROR(lParam);
			if (rc != 0) {
				Clear_Socket_Error (Socket);
				return (0);;
			}

			/*
			** Call the Winsock recvfrom function to get the outstanding packet.
			*/
			addr_len = sizeof(addr);
			rc = recvfrom ( Socket, (char*)ReceiveBuffer, sizeof (ReceiveBuffer), 0, (LPSOCKADDR)&addr, &addr_len);
			if (rc == SOCKET_ERROR) {
				Clear_Socket_Error (Socket);
				return (0);;
			}

			/*
			** rc is the number of bytes received from Winsock
			*/
			if ( rc ) {

				/*
				** Make sure this packet didn't come from us. If it did then throw it away.
				*/
				for ( int i=0 ; i<LocalAddresses.Count() ; i++ ) {
					if ( ! memcmp (LocalAddresses[i], &addr.sin_addr.s_addr, 4) ) return (0);
				}

				/*
				** Create a new buffer and store this packet in it.
				*/
				packet = new WinsockBufferType;
				packet->BufferLen = rc;
				memcpy ( packet->Buffer, ReceiveBuffer, rc );
				memset ( packet->Address, 0, sizeof (packet->Address) );
				memcpy ( packet->Address+4, &addr.sin_addr.s_addr, 4 );
				InBuffers.Add (packet);
			}
			return (0);


		/*
		** Write event. We send ourselves this event when we have more data to send. This
		** event will also occur automatically when a packet has finished being sent.
		*/
		case FD_WRITE:
			/*
			** Clear any outstanding erros on the socket.
			*/
			rc = WSAGETSELECTERROR(lParam);
			if (rc != 0) {
				Clear_Socket_Error (Socket);
				return (0);;
			}

			/*
			** If there are no packets waiting to be sent then bail.
			*/
			if ( OutBuffers.Count() == 0 ) return (0);
			int packetnum = 0;

			/*
			** Get a pointer to the packet.
			*/
			packet = OutBuffers [ packetnum ];

			/*
			** Set up the address structure of the outgoing packet
			*/
			addr.sin_family = AF_INET;
			addr.sin_port = (unsigned short) htons ((unsigned short)PlanetWestwoodPortNumber);
			memcpy (&addr.sin_addr.s_addr, packet->Address+4, 4);

			/*
			** Send it.
			** If we get a WSAWOULDBLOCK error it means that Winsock is unable to accept the packet
			** at this time. In this case, we clear the socket error and just exit. Winsock will
			** send us another WRITE message when it is ready to receive more data.
			*/
			rc = sendto ( Socket, (const char*) packet->Buffer, packet->BufferLen, 0, (LPSOCKADDR)&addr, sizeof (addr) );

			if (rc == SOCKET_ERROR){
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					Clear_Socket_Error (Socket);
					return (0);
				}
			}

			/*
			** Delete the sent packet.
			*/
			OutBuffers.Delete ( packetnum );
			delete packet;
			return(0);
	}

	return (0);
}
#endif