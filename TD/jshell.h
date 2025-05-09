/*
**	Command & Conquer(tm)
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

/* $Header:   F:\projects\c&c\vcs\code\jshell.h_v   2.16   16 Oct 1995 16:45:06   JOE_BOSTIC  $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                    File Name : JSHELL.H                                                     * 
 *                                                                                             * 
 *                   Programmer : Joe L. Bostic                                                * 
 *                                                                                             * 
 *                   Start Date : 03/13/95                                                     * 
 *                                                                                             * 
 *                  Last Update : March 13, 1995 [JLB]                                         * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef JSHELL_H
#define JSHELL_H

#include <stdint.h>

/*
**	Interface class to the keyboard. This insulates the game from library vagaries. Most
**	notable being the return values are declared as "int" in the library whereas C&C
**	expects it to be of KeyNumType.
*/
class Keyboard 
{
	public:
		static KeyNumType Get(void) {return (KeyNumType)Get_Key_Num();};
		static KeyNumType Check(void) {return (KeyNumType)Check_Key_Num();};
		static KeyASCIIType To_ASCII(KeyNumType key) {return (KeyASCIIType)KN_To_KA(key);};
		static void Clear(void) {Clear_KeyBuffer();};
		static void Stuff(KeyNumType key) {Stuff_Key_Num(key);};
		static int Down(KeyNumType key) {return Key_Down(key);};
		static int Mouse_X(void) {return Get_Mouse_X();};
		static int Mouse_Y(void) {return Get_Mouse_Y();};
};


#ifdef NEVER
inline void * operator delete(void * data) 
{
	Free(data);
}

inline void * operator delete[] (void * data)
{
	Free(data);
}
#endif


/*
**	These templates allow enumeration types to have simple bitwise
**	arithmatic performed. The operators must be instatiated for the
**	enumerated types desired.
*/
template<class T> inline T operator ++(T & a)
{
	a = (T)((int)a + (int)1);
	return(a);
}
template<class T> inline T operator ++(T & a, int)
{
	T aa = a;
	a = (T)((int)a + (int)1);
	return(aa);
}	
template<class T> inline T operator --(T & a)
{
	a = (T)((int)a - (int)1);
	return(a);
}
template<class T> inline T operator --(T & a, int)
{
	T aa = a;
	a = (T)((int)a - (int)1);
	return(aa);
}
template<class T> inline constexpr T operator |(T t1, T t2)
{
	return((T)((int)t1 | (int)t2));
}
template<class T> inline T operator &(T t1, T t2)
{
	return((T)((int)t1 & (int)t2));
}
template<class T> inline T operator ~(T t1)
{
	return((T)(~(int)t1));
}


inline void Set_Bit(void * array, int bit, int value)
{
/*
#pragma aux Set_Bit parm [esi] [ecx] [eax] \
	modify [esi ebx] = 			\
	"mov	ebx,ecx"					\
	"shr	ebx,5"					\
	"and	ecx,01Fh"				\
	"btr	[esi+ebx*4],ecx"		\
	"or	eax,eax"					\
	"jz	ok"						\
	"bts	[esi+ebx*4],ecx"		\
	"ok:"
*/
    if(value)
        ((uint32_t *)array)[(unsigned)bit >> 5] |= (1 << (bit & 0x1F));
    else
        ((uint32_t *)array)[(unsigned)bit >> 5] &= ~(1 << (bit & 0x1F));
}

inline int Get_Bit(void const * array, int bit)
{
/*
	"mov	ebx,eax"					\
	"shr	ebx,5"					\
	"and	eax,01Fh"				\
	"bt	[esi+ebx*4],eax"		\
	"setc	al"
*/
    return !!(((const uint32_t *)array)[(unsigned)bit >> 5] & (1 << (bit & 0x1F)));
}

inline int First_True_Bit(void const * array)
{
/*
#pragma aux First_True_Bit parm [esi] \
	modify [esi ebx] \
	value [eax]		= 				\
	"mov	eax,-32"					\
	"again:"							\
	"add	eax,32"					\
	"mov	ebx,[esi]"				\
	"add	esi,4"					\
	"bsf	ebx,ebx"					\
	"jz	again"					\
	"add	eax,ebx"
*/
    const uint32_t *array32 = (const uint32_t *)array;
    int off = 0;
    while(true)
    {
        uint32_t v = *array32++;
#ifdef _MSC_VER
		DWORD pos;
		if(_BitScanForward(&pos, v))
			return off + pos;
#else
        int pos = __builtin_ffs(v);
        if(pos)
            return off + pos - 1;
#endif
        off += 32;
    }
}
inline int First_False_Bit(void const * array)
{
/*
#pragma aux First_False_Bit parm [esi] \
	modify [esi ebx] \
	value [eax]		= 				\
	"mov	eax,-32"					\
	"again:"							\
	"add	eax,32"					\
	"mov	ebx,[esi]"				\
	"not	ebx"						\
	"add	esi,4"					\
	"bsf	ebx,ebx"					\
	"jz	again"					\
	"add	eax,ebx"
*/
    const uint32_t *array32 = (const uint32_t *)array;
    int off = 0;
    while(true)
    {
        uint32_t v = *array32++;
#ifdef _MSC_VER
		DWORD pos;
		if(_BitScanForward(&pos, ~v))
			return off + pos;
#else
        int pos = __builtin_ffs(~v);
        if(pos)
            return off + pos - 1;
#endif
        off += 32;
    }
}

#ifdef PORTABLE
inline int Bound(int original, int minval, int maxval)
{
	if (original < minval) return(minval);
	if (original > maxval) return(maxval);
	return(original);
};
#else
extern int Bound(int original, int min, int max);
#pragma aux Bound parm [eax] [ebx] [ecx] \
	modify [eax] \
	value [eax]		= 				\
	"cmp	ebx,ecx"					\
	"jl	okorder"					\
	"xchg	ebx,ecx"					\
	"okorder: cmp	eax,ebx"		\
	"jg	okmin"					\
	"mov	eax,ebx"					\
	"okmin: cmp	eax,ecx"			\
	"jl	okmax"					\
	"mov	eax,ecx"					\
	"okmax:"
#endif

#ifdef NEVER
extern unsigned Bound(unsigned original, unsigned min, unsigned max);
#pragma aux Bound parm [eax] [ebx] [ecx] \
	modify [eax] \
	value [eax]		= 				\
	"cmp	ebx,ecx"					\
	"jb	okorder"					\
	"xchg	ebx,ecx"					\
	"okorder: cmp	eax,ebx"		\
	"ja	okmin"					\
	"mov	eax,ebx"					\
	"okmin: cmp	eax,ecx"			\
	"jb	okmax"					\
	"mov	eax,ecx"					\
	"okmax:"
#endif


unsigned Fixed_To_Cardinal(unsigned base, unsigned fixed);
#pragma aux Fixed_To_Cardinal parm [eax] [edx] \
	modify [edx] \
	value [eax]		= 				\
	"mul	edx"						\
	"add	eax,080h"				\
	"test	eax,0FF000000h"		\
	"jz	ok"						\
	"mov	eax,000FFFFFFh"		\
	"ok:"								\
	"shr	eax,8"


unsigned Cardinal_To_Fixed(unsigned base, unsigned cardinal);
#pragma aux Cardinal_To_Fixed parm [ebx] [eax] \
	modify [edx] \
	value [eax]		= 				\
	"or	ebx,ebx"					\
	"jz	fini"						\
	"shl	eax,8"					\
	"xor	edx,edx"					\
	"div	ebx"						\
	"fini:"

#endif
