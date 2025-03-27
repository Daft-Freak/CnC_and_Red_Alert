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

/* $Header: /CounterStrike/CCINI.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CCINI.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/24/96                                                     *
 *                                                                                             *
 *                  Last Update : May 24, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef CCINI_H
#define CCINI_H

#include	"ini.h"
#include	"fixed.h"
#include	"pk.h"

class TriggerTypeClass;

/*
**	The advanced version of the INI database manager. It handles the C&C expansion types and
**	identifiers. In addition, it automatically stores a message digest with the INI data
**	so that verification can occur.
*/
class CCINIClass
{
	public:
		CCINIClass(void) : IsDigestPresent(false), Buffer(NULL) {}
		~CCINIClass();

		int Load(FileClass & file, bool withdigest);
		int Load(Straw & file, bool withdigest);
		int Save(FileClass & file, bool withdigest) const;
		int Save(Pipe & pipe, bool withdigest) const;

		bool Clear(char const * section = 0, char const * entry = 0);
		
		bool Is_Present(char const * section, char const * entry = 0) const;

		int Entry_Count(char const * section) const;
		char const * Get_Entry(char const * section, int index) const;

		int Get_String(char const * section, char const * entry, char const * defvalue, char * buffer, int size) const;
		int Get_Int(char const * section, char const * entry, int defvalue=0) const;
		bool Get_Bool(char const * section, char const * entry, bool defvalue=false) const;
		fixed Get_Fixed(char const * section, char const * entry, fixed defvalue) const;
		int Get_TextBlock(char const * section, char * buffer, int len) const;
		int Get_UUBlock(char const * section, void * buffer, int len) const;

		bool Put_Fixed(char const * section, char const * entry, fixed value);
		bool Put_String(char const * section, char const * entry, char const * string);
		bool Put_Int(char const * section, char const * entry, int number, int format=0);
		bool Put_Bool(char const * section, char const * entry, bool value);
		bool Put_UUBlock(char const * section, void const * block, int len);

		long Get_Buildings(char const * section, char const * entry, long defvalue) const;
		UnitType Get_UnitType(char const * section, char const * entry, UnitType defvalue) const;
		AnimType Get_AnimType(char const * section, char const * entry, AnimType defvalue) const;
		ArmorType Get_ArmorType(char const * section, char const * entry, ArmorType defvalue) const;
		BulletType Get_BulletType(char const * section, char const * entry, BulletType defvalue) const;
		HousesType Get_HousesType(char const * section, char const * entry, HousesType defvalue) const;
		LEPTON Get_Lepton(char const * section, char const * entry, LEPTON defvalue) const;
		MPHType Get_MPHType(char const * section, char const * entry, MPHType defvalue) const;
		OverlayType Get_OverlayType(char const * section, char const * entry, OverlayType defvalue) const;
		SourceType Get_SourceType(char const * section, char const * entry, SourceType defvalue) const;
		TerrainType Get_TerrainType(char const * section, char const * entry, TerrainType defvalue) const;
		TheaterType Get_TheaterType(char const * section, char const * entry, TheaterType defvalue) const;
		ThemeType Get_ThemeType(char const * section, char const * entry, ThemeType defvalue) const;
		TriggerTypeClass * Get_TriggerType(char const * section, char const * entry) const;
		VQType Get_VQType(char const * section, char const * entry, VQType defvalue) const;
		VocType Get_VocType(char const * section, char const * entry, VocType defvalue) const;
		WarheadType Get_WarheadType(char const * section, char const * entry, WarheadType defvalue) const;
		WeaponType Get_WeaponType(char const * section, char const * entry, WeaponType defvalue) const;
		long Get_Owners(char const * section, char const * entry, long defvalue) const;
		CrateType Get_CrateType(char const * section, char const * entry, CrateType defvalue) const;


		bool Put_Buildings(char const * section, char const * entry, long value);
		bool Put_AnimType(char const * section, char const * entry, AnimType value);
		bool Put_UnitType(char const * section, char const * entry, UnitType value);
		bool Put_ArmorType(char const * section, char const * entry, ArmorType value);
		bool Put_BulletType(char const * section, char const * entry, BulletType value);
		bool Put_HousesType(char const * section, char const * entry, HousesType value);
		bool Put_Lepton(char const * section, char const * entry, LEPTON value);
		bool Put_MPHType(char const * section, char const * entry, MPHType value);
		bool Put_VQType(char const * section, char const * entry, VQType value);
		bool Put_OverlayType(char const * section, char const * entry, OverlayType value);
		bool Put_Owners(char const * section, char const * entry, long value);
		bool Put_SourceType(char const * section, char const * entry, SourceType value);
		bool Put_TerrainType(char const * section, char const * entry, TerrainType value);
		bool Put_TheaterType(char const * section, char const * entry, TheaterType value);
		bool Put_ThemeType(char const * section, char const * entry, ThemeType value);
		bool Put_TriggerType(char const * section, char const * entry, TriggerTypeClass * value);
		bool Put_VocType(char const * section, char const * entry, VocType value);
		bool Put_WarheadType(char const * section, char const * entry, WarheadType value);
		bool Put_WeaponType(char const * section, char const * entry, WeaponType value);
		bool Put_CrateType(char const * section, char const * entry, CrateType value);

		int Get_Unique_ID(void) const;

	private:
		struct SIFHeader {
			uint16_t Flags;
			uint16_t SectionCount;
			uint16_t TotalEntryCount;
			uint16_t StringTableSize;
		};

		struct SIFSection {
			uint16_t EntryCount;
			uint16_t NameIndex;
		};

		struct SIFEntry {
			uint16_t KeyIndex;
			uint16_t ValueIndex;
		};

		void Calculate_Message_Digest(void);
		void Invalidate_Message_Digest(void);

		char const *Lookup_String(uint16_t offset) const;

		SIFSection *Find_Section(char const *name) const;
		SIFSection *Find_Section(char const *name, int &entryoffset) const;

		SIFEntry *Find_Entry(char const *section, char const *name) const;
		SIFEntry *Find_Entry(char const *name, SIFEntry *start, int numentries) const;

		char const *Find_Entry_Value(char const *section, char const *name) const;

		bool IsDigestPresent:1;

		/*
		**	This is the message digest (SHA) of the INI database that was embedded as part of
		**	the INI file.
		*/
		unsigned char Digest[20];

		INIClass ini;

		uint8_t *Buffer;
	
		SIFSection *Sections;
		SIFEntry *Entries;
		const char *Strings;

		uint16_t StringTableSize;
};

#endif
