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

/* $Header: /CounterStrike/CSTRAW.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CSTRAW.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/10/96                                                     *
 *                                                                                             *
 *                  Last Update : November 10, 1996 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef CSTRAW_H
#define CSTRAW_H

#include	"straw.h"
#include	"buff.h"

/*
**	This class handles transfer of data by perform regulated requests for data from the next
**	class in the chain. It performs no translation on the data. By using this segment in a
**	straw chain, data throughput can be regulated. This can yield great performance increases
**	when dealing with a file source.
*/
class CacheStraw : public Straw
{
	public:
		CacheStraw(Buffer const & buffer) : BufferPtr(buffer), Index(0), Length(0) {}
		CacheStraw(int length=4096) : BufferPtr(length), Index(0), Length(0) {}
		virtual int Get(void * source, int slen);

	private:
		Buffer BufferPtr;
		int Index;
		int Length;

		bool Is_Valid(void) {return(BufferPtr.Is_Valid());}
		CacheStraw(CacheStraw & rvalue);
		CacheStraw & operator = (CacheStraw const & pipe);
};



#endif

