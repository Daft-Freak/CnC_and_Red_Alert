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

/* $Header:   F:\projects\c&c\vcs\code\function.h_v   2.21   16 Oct 1995 16:46:44   JOE_BOSTIC  $*/
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FUNCTION.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 27, 1994                                                 *
 *                                                                                             *
 *                  Last Update : May 27, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef FUNCTION_H
#define FUNCTION_H

/*
Map (screen) class heirarchy.

 MapeditClass (most derived class) -- scenario editor
        │
   MouseClass -- handles mouse animation and display control
        │
  ScrollClass -- map scroll handler
        │
    HelpClass -- pop-up help text handler
        │
     TabClass -- file folder tab screen mode control dispatcher
        │
 SidebarClass -- displays and controls construction list sidebar
        │
   PowerClass -- display power production/consumption bargraph
        │
   RadarClass -- displays and controls radar map
        │
 DisplayClass -- general tactical map display handler
        │
     MapClass -- general tactical map data handler
        │
 GScreenClass (pure virtual base class) -- generic screen control

                          AbstractClass
                                  │
                                  │
                                  │
                                  │
                            ObjectClass
                                  │
       ┌──────┬──────────┬────────┼────────┬────────────────┬───────────┐
   AnimClass  │  TemplateClass    │        ├─ FuseClass     │    TerrainClass
              │                   │        ├─ FlyClass      │
              │                   │  BulletClass            │
       OverlayClass        MissionClass               SmudgeClass
                                  │
                             RadioClass
                                  │
                                  ├─ CrewClass
                                  ├─ FlasherClass
                                  ├─ StageClass
                                  ├─ CargoClass
                            TechnoClass
                                  │
         ┌────────────────────────┴────────────────────────────┐
     FootClass                                         BuildingClass
         │
         ├──────────────┬─────────────┐
    DriveClass  InfantryClass         ├─ FlyClass
         │                      AircraftClass
   TurretClass
         │
   TarComClass
         │
     UnitClass


                            AbstractTypeClass
                                    │
                              ObjectTypeClass
                                    │
             ┌──────────────────────┼────────────┬─────────────────┐
             │                      │            │                 │
       TechnoTypeClass              │            │                 │
             │                BulletTypeClass    │                 │
             │                           TemplateTypeClass         │
    ┌────────┴─────┬───────────┬──────────────┐             TerrainTypeClass
    │              │           │              │
UnitTypeClass      │   BuildingTypeClass      │
                   │                  InfantryTypeClass
           AircraftTypeClass
*/



#ifdef _WIN32
#include <windows.h>
#endif

/**********************************************************************
**	If the following define is enabled, then the memory checking code
**	will be disabled.
*/
#define NOMEMCHECK

#include	"watcom.h"

/**********************************************************************
**	This class is solely used as a parameter to a constructor that does
**	absolutely no initialization to the object being constructed. By using
**	this method, it is possible to load and save data directly from a
**	class that has virtual functions. The construction process automatically
**	takes care of initializing the virtual function table pointer and the
**	rest of the constructor doesn't initialize any data members. After loading
**	into a class object, simply perform an in-place new operation.
*/
#ifndef NOINITCLASS
#define NOINITCLASS
struct NoInitClass {
	public:
		void operator () (void) const {};
};
#endif

#ifdef PORTABLE
#include "keyboard.h"
#endif

#define WWMEM_H
#include "compat.h"
#include <wwlib32.h>
#include	"jshell.h"



// Should be part of WWLIB.H. This is used in JSHELL.CPP.
typedef struct {
	unsigned char	SourceColor;
	unsigned char	DestColor;
	unsigned char	Fading;
	unsigned char	reserved;
} TLucentType;


// Don't complain if these headers aren't referenced.
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<stddef.h>
#ifdef PORTABLE
#include    "ex_string.h"
#else
#include	<mem.h>
#include	<dos.h>
#include	<direct.h>
#include	<process.h>
#endif
#include	<stdarg.h>
#include	<ctype.h>
#include	<assert.h>
#include	<new>

/*
**	VQ player specific includes.
*/
#include <vqa32/vqaplay.h>
#include <vqa32/vqafile.h>

extern bool GameActive;
extern long LParam;

#include	"vector.h"
#include	"heap.h"
#include	"ccfile.h"
#include	"monoc.h"
#include	"conquer.h"
//#include	"debug.h"
#include "special.h"
#include	"defines.h"

#ifndef PORTABLE
/*
**	Greenleaf specific includes.
*/
#include <modem.h>
#include <fast.h>
#endif


extern long Frame;
inline CELL Coord_XCell(COORDINATE coord) {return (CELL)(*(((unsigned char*)&coord)+1));}
inline CELL Coord_YCell(COORDINATE coord) {return (CELL)(*(((unsigned char*)&coord)+3));}
#ifdef PORTABLE
inline CELL Coord_Cell(COORDINATE coord){return (CELL)(((*(((unsigned short *)&coord)+1) & 0xFF00) >> 2) | *(((unsigned char *)&coord)+1));}
#else
CELL Coord_Cell(COORDINATE coord);
#pragma aux Coord_Cell parm [eax] \
	modify [ebx] \
	value [ax]		= 				\
	"mov	ebx,eax"					\
	"shr	eax,010h"				\
	"xor	al,al"					\
	"shr	eax,2"					\
	"or	al,bh"
#endif

#include  "utracker.h"
#include	"palette.h"
#include	"facing.h"
#include	"ftimer.h"
#include	"theme.h"
#include	"link.h"
#include	"gadget.h"
#include	"control.h"
#include	"toggle.h"
#include	"checkbox.h"
#include	"shapebtn.h"
#include	"textbtn.h"
#include	"slider.h"
#include "list.h"
#include "cheklist.h"
#include "colrlist.h"
#include	"edit.h"
#include	"gauge.h"
#include	"msgbox.h"
#include	"dial8.h"
#include	"txtlabel.h"
#include	"super.h"
#include	"house.h"
#include	"gscreen.h"
#include	"map.h"
#include	"display.h"
#include	"radar.h"
#include	"power.h"
#include	"sidebar.h"
#include	"tab.h"
#include	"help.h"
#include	"mouse.h"
//#include	"mapedit.h"
#include	"help.h"
#include	"target.h"
#include	"theme.h"
#include	"team.h"				// Team objects.
#include	"teamtype.h"		// Team type objects.
#include	"trigger.h"			// Trigger event objects.
#include	"mapedit.h"			// ???
#include	"abstract.h"
#include "object.h"
#include "mission.h"
#include	"door.h"
#include	"bullet.h"			// Bullet objects.
#include	"terrain.h"			// Terrain objects.
#include	"anim.h"				// Animation objects.
#include	"template.h"		// Icon template objects.
#include	"overlay.h"			// Overlay objects.
#include	"smudge.h"			// Stains on the terrain objects.
#include	"aircraft.h"		// Aircraft objects.
#include	"unit.h"				// Ground unit objects.
#include	"infantry.h"		// Infantry objects.
#include	"credits.h"			// Credit counter class.
#include	"score.h"			// Scoring system class.
#include	"factory.h"			// Production manager class.
#include "intro.h"
#include "ending.h"
#include	"logic.h"
#include	"queue.h"
#include	"event.h"
#include "base.h"				// defines the AI's pre-built base
#include "ipxmgr.h"
#include	"combuf.h"
#include	"connect.h"
#include	"connmgr.h"
#include	"noseqcon.h"
#include "msglist.h"
#include	"nullconn.h"
#include	"nullmgr.h"
#include "phone.h"
#include "loaddlg.h"
#include	"ipxaddr.h"
/****************************************************************************
**	This is a "node", used for the lists of available games & players.  The
**	'Game' structure is used for games; the 'Player' structure for players.
*/
typedef struct NodeNameTag {
	char Name[MPLAYER_NAME_MAX];
	IPXAddressClass Address;
	union {
		struct {
			int Version;
			unsigned char IsOpen;
			unsigned long LastTime;
		} Game;
		struct {
			HousesType House;
			unsigned char Color;
		} Player;
	};
} NodeNameType;


#include	"externs.h"


extern int Get_CD_Drive(void);
extern void Fatal(char const *message, ...);


/*
**	ANIM.CPP
*/
void Shorten_Attached_Anims(ObjectClass * obj);

/*
**	AUDIO.CPP
*/
int Sound_Effect(VocType voc, VolType volume, int variation=1, signed short panvalue=0);
void Speak(VoxType voice);
void Speak_AI(void);
void Stop_Speaking(void);
void Sound_Effect(VocType voc, COORDINATE coord=0, int variation=1);
bool Is_Speaking(void);

/*
**	COMBAT.CPP
*/
int Modify_Damage(int damage, WarheadType warhead, ArmorType armor, int distance);
void Explosion_Damage(COORDINATE coord, unsigned strength, TechnoClass * source, WarheadType warhead);

/*
**	CONQUER.CPP
*/
void Center_About_Objects(void);
bool Force_CD_Available(int cd);
void Handle_View(int view, int action=0);
void Handle_Team(int team, int action=0);
TechnoTypeClass const * Fetch_Techno_Type(RTTIType type, int id);
char const * Fading_Table_Name(char const * base, TheaterType theater);
void Unselect_All(void);
void Play_Movie(char const * name, ThemeType theme=THEME_NONE, bool clrscrn=true);
bool Main_Loop();
TheaterType Theater_From_Name(char const *name);
//DirType Rotation_Calc(DirType current, DirType desired, int rate);
void Main_Game(int argc, char *argv[]);
long VQ_Call_Back(unsigned char * buffer=NULL, long frame=0);
long VQ_Event_Handler(unsigned long event, void *buffer, long nbytes);
void Call_Back(void);
char const *Language_Name(char const *basename);
SourceType Source_From_Name(char const *name);
char const *Name_From_Source(SourceType source);
FacingType KN_To_Facing(int input);
void const *Get_Radar_Icon(void const *shapefile, int shapenum, int frames, int zoomfactor);
void CC_Draw_Shape(void const * shapefile, int shapenum, int x, int y, WindowNumberType window, ShapeFlags_Type flags, void const * fadingdata=0, void const * ghostdata=0);
void Go_Editor(bool flag);
long MixFileHandler(VQAHandle *vqa, long action, void *buffer, long nbytes);

char *CC_Get_Shape_Filename(void const *shapeptr );
void CC_Add_Shape_To_Global(void const *shapeptr, char *filename, char code );

void Bubba_Print(char *format,...);

void Heap_Dump_Check( char *string );
void Dump_Heap_Pointers( void );

void Validate_Error(char *name);
void const * Hires_Retrieve(char *name);
int Get_Resolution_Factor(void);


/*
** INTERPAL.CPP
*/
#define SIZE_OF_PALETTE 256
extern	"C" unsigned char *InterpolationPalette;
extern	bool	InterpolationPaletteChanged;
extern	void 	Interpolate_2X_Scale( GraphicBufferClass *source, GraphicViewPortClass *dest ,char const *palette_file_name);
void Read_Interpolation_Palette (char const *palette_file_name);
void Write_Interpolation_Palette (char const *palette_file_name);
void Increase_Palette_Luminance(unsigned char *InterpolationPalette ,	int RedPercentage ,int GreenPercentage ,int BluePercentage ,int cap);
extern "C"{
	extern unsigned char PaletteInterpolationTable[SIZE_OF_PALETTE][SIZE_OF_PALETTE];
	extern unsigned char *InterpolationPalette;
	void __cdecl Asm_Create_Palette_Interpolation_Table(void);
}


/*
**	COORD.CPP
*/
void  Move_Point(short &x, short &y, register DirType dir, unsigned short distance);
COORDINATE  Adjacent_Cell(COORDINATE coord, FacingType dir);
COORDINATE  Coord_Move(COORDINATE start, DirType facing, unsigned short distance);
COORDINATE  Coord_Scatter(COORDINATE coord, unsigned distance, bool lock=false);
DirType  Direction(CELL cell1, CELL cell2);
DirType  Direction(COORDINATE coord1, COORDINATE coord2);
DirType  Direction256(COORDINATE coord1, COORDINATE coord2);
DirType  Direction8(COORDINATE coord1, COORDINATE coord2);
int  Distance(CELL coord1, CELL coord2);
int  Distance(COORDINATE coord1, COORDINATE coord2);
short const *  Coord_Spillage_List(COORDINATE coord, int maxsize);
//void Move_Point(unsigned short &x, unsigned short &y, DirType dir, unsigned short distance);

/*
**	COORDA.CPP
*/
//extern "C" {
//unsigned Cardinal_To_Fixed(unsigned base, unsigned cardinal);
//unsigned Fixed_To_Cardinal(unsigned base, unsigned fixed);
//}

/*
**	DEBUG.CPP
*/
void Log_Event(char const *text, ...);
void Debug_Key(unsigned input);
void Self_Regulate(void);

/*
**	DIALOG.CPP
*/
int Format_Window_String(char * string, int maxlinelen, int & width, int & height);
extern void Dialog_Box(int x, int y, int w, int h);
void Conquer_Clip_Text_Print(char const *, unsigned x, unsigned y, unsigned fore, unsigned back=(unsigned)TBLACK, TextPrintType flag=TPF_8POINT|TPF_DROPSHADOW, unsigned width=-1, int const * tabs=0);
void Draw_Box(int x, int y, int w, int h, BoxStyleEnum up, bool filled);
int __cdecl Dialog_Message(char *errormsg, ...);
void Window_Box(WindowNumberType window, BoxStyleEnum style);
void Fancy_Text_Print(char const *text, unsigned x, unsigned y, unsigned fore, unsigned back, TextPrintType flag, ...);
void Fancy_Text_Print(int text, unsigned x, unsigned y, unsigned fore, unsigned back, TextPrintType flag, ...);
void Simple_Text_Print(char const *text, unsigned x, unsigned y, unsigned fore, unsigned back, TextPrintType flag);

/*
**	DISPLAY.CPP
*/

/*
**	ENDING.CPP
*/
void GDI_Ending(void);
void Nod_Ending(void);

/*
**	EXPAND.CPP
*/
bool Expansion_Present(void);
bool Expansion_Dialog(void);
bool Bonus_Dialog(void);

/*
**	FINDPATH.CPP
*/
//PathType * Find_Path(CELL source, CELL dest, FacingType *final_moves, int maxlen, int (*callback)(CELL, FacingType), int threshhold);
int Optimize_Moves(PathType *path, int (*callback)(CELL, FacingType), int threshhold);

/*
**	GOPTIONS.CPP
*/
void Draw_Caption(int text, int x, int y, int w);

/*
**	INI.CPP
*/
void Set_Scenario_Name(char *buf, int scenario, ScenarioPlayerType player, ScenarioDirType dir = SCEN_DIR_NONE, ScenarioVarType var = SCEN_VAR_NONE);
void Write_Scenario_Ini(char *root);
bool Read_Scenario_Ini(char *root, bool fresh=true);
int Scan_Place_Object(ObjectClass *obj, CELL cell);

/*
**	INIT.CPP
*/
void Uninit_Game(void);
void Load_Title_Page(bool visible=false);
long Obfuscate(char const * string);
void Anim_Init(void);
bool Init_Game(int argc, char *argv[]);
bool Select_Game(bool fade = false);
bool Parse_Command_Line(int argc, char *argv[]);
void Parse_INI_File(void);
int Version_Number(void);
void Save_Recording_Values(void);
void Load_Recording_Values(void);

/*
** JSHELL.CPP
*/
int Load_Picture(char const *filename, BufferClass& scratchbuf, BufferClass& destbuf, unsigned char *palette, PicturePlaneType format);
void * Small_Icon(void const * iconptr, int iconnum);
void Set_Window(int window, int x, int y, int w, int h);
void * Load_Alloc_Data(FileClass &file);
long Load_Uncompress(FileClass &file, BuffType &uncomp_buff, BuffType &dest_buff, void *reserved_data);
long Translucent_Table_Size(int count);
void *Build_Translucent_Table(void const *palette, TLucentType const *control, int count, void *buffer);
void *Conquer_Build_Translucent_Table(void const *palette, TLucentType const *control, int count, void *buffer);

/*
**	KEYFBUFF.ASM
*/
#ifdef __cplusplus
extern "C" {
#endif
long __cdecl Buffer_Frame_To_Page(int x, int y, int w, int h, void *Buffer, GraphicViewPortClass &view, int flags, ...);
#ifdef __cplusplus
}
#endif

/*
**	KEYFRAME.CPP
*/
int Get_Last_Frame_Length(void);
void *Build_Frame(void const *dataptr, unsigned short framenumber, void *buffptr);
unsigned short Get_Build_Frame_Count(void const *dataptr);
unsigned short Get_Build_Frame_X(void const *dataptr);
unsigned short Get_Build_Frame_Y(void const *dataptr);
unsigned short Get_Build_Frame_Width(void const *dataptr);
unsigned short Get_Build_Frame_Height(void const *dataptr);
bool Get_Build_Frame_Palette(void const *dataptr, void *palette);

/*
**	MAP.CPP
*/
int Terrain_Cost(CELL cell, FacingType facing);
int Coord_Spillage_Number(COORDINATE coord, int maxsize);

/*
**	MENUS.CPP
*/
void Setup_Menu(int menu, char const *text[], unsigned long field, int index, int skip);
int Check_Menu(int menu, char const *text[], char *selection, long field, int index);
int Do_Menu(char const **strings, bool blue);
extern int UnknownKey;
int Main_Menu(unsigned long timeout);

/*
** MPLAYER.CPP
*/
GameType Select_MPlayer_Game (void);
void Read_MultiPlayer_Settings (void);
void Write_MultiPlayer_Settings (void);
void Read_Scenario_Descriptions (void);
void Free_Scenario_Descriptions(void);
void Computer_Message(void);
int Surrender_Dialog(void);

/*
** NETDLG.CPP
*/
bool Init_Network (void);
void Shutdown_Network (void);
bool Remote_Connect (void);
void Destroy_Connection(int id, int error);
bool Process_Global_Packet(GlobalPacketType *packet, IPXAddressClass *address);
unsigned long Compute_Name_CRC(char *name);
void Net_Reconnect_Dialog(int reconn, int fresh, int oldest_index, unsigned long timeval);

/*
** NULLDLG.CPP
*/
int Init_Null_Modem( SerialSettingsType *settings );
void Shutdown_Modem( void );
void Modem_Signoff( void );
int Test_Null_Modem( void );
int Reconnect_Modem( void );
void Destroy_Null_Connection(int id, int error);
GameType Select_Serial_Dialog( void );
int Com_Scenario_Dialog(void);
int Com_Show_Scenario_Dialog(void);

void Smart_Printf( char *format, ... );
void Hex_Dump_Data( char *buffer, int length );
void itoh( int i, char *s);

/*
**	PROFILE.CPP
*/
int WWGetPrivateProfileInt(char const *section, char const *entry, int def, char *profile);
bool WWWritePrivateProfileInt(char const *section, char const *entry, int value, char *profile);
bool WWWritePrivateProfileString(char const *section, char const *entry, char const *string, char *profile);
char * WWGetPrivateProfileString(char const *section, char const *entry, char const *def, char *retbuffer, int retlen, char *profile);
unsigned WWGetPrivateProfileHex (char const *section, char const *entry, char *profile);

/*
** QUEUE.CPP
*/
bool Queue_Target(TARGET whom, TARGET target);
bool Queue_Destination(TARGET whom, TARGET target);
bool Queue_Mission(TARGET whom, MissionType mission);
bool Queue_Mission(TARGET whom, MissionType mission, TARGET target, TARGET destination);
bool Queue_Options(void);
bool Queue_Exit(void);
void Queue_AI(void);
void Add_CRC(unsigned long *crc, unsigned long val);

/*
**	RAND.CPP
*/
int Sim_IRandom(int minval, int maxval);
int Sim_Random(void);

/*
**	REINF.CPP
*/
bool Do_Reinforcements(TeamTypeClass *team);
bool Create_Special_Reinforcement(HouseClass * house, TechnoTypeClass const * type, TechnoTypeClass const * another, TeamMissionType mission = TMISSION_NONE, int argument =0);
int Create_Air_Reinforcement(HouseClass *house, AircraftType air, int number, MissionType mission, TARGET tarcom, TARGET navcom);

/*
**	SAVELOAD.CPP
*/
bool Load_Misc_Values(FileClass &file);
bool Save_Misc_Values(FileClass &file);
bool Get_Savefile_Info(int id, char *buf, unsigned *scenp, HousesType *housep);
bool Load_Game(int id);
bool Read_Object (void *ptr, int base_size, int class_size, FileClass & file, void * vtable);
bool Save_Game(int id,char *descr);
bool Write_Object (void *ptr, int class_size, FileClass & file);
TARGET TechnoType_To_Target(TechnoTypeClass const * ptr);
TechnoTypeClass const * Target_To_TechnoType(TARGET target);
void * Get_VTable(void *ptr, int base_size);
void Code_All_Pointers(void);
void Decode_All_Pointers(void);
void Dump(void);
void Set_VTable(void *ptr, int base_size, void *vtable);

/*
** SCENARIO.CPP
*/
bool End_Game(void);
bool Read_Scenario(char *root);
bool Start_Scenario(char *root, bool briefing=true);
HousesType Select_House(void);
void Clear_Scenario(void);
void Do_Briefing(char const * text);
void Do_Lose(void);
void Do_Win(void);
void Do_Restart(void);
void Fill_In_Data(void);
bool Restate_Mission(char const * name, int button1, int button2);

/*
**	SCORE.CPP
*/
void Map_Selection(void);
void Bit_It_In_Scale(int x, int y, int w, int h, GraphicBufferClass *src, GraphicBufferClass *dest, GraphicViewPortClass *seen , int delay=0, int dagger=0);
void Bit_It_In(int x, int y, int w, int h, GraphicBufferClass *src, GraphicBufferClass *dest, int delay=0, int dagger=0);
void Call_Back_Delay(int time);
int Alloc_Object(ScoreAnimClass *obj);
#ifndef LORES
extern GraphicBufferClass *PseudoSeenBuff;
#endif

#ifndef PORTABLE
void Window_Dialog_Box(HANDLE  hinst, LPCTSTR  lpszTemplate, HWND  hwndOwner, DLGPROC  dlgprc);
#endif

/*
**	SPECIAL.CPP
*/
void Special_Dialog(void);

/*
**	SUPPORT.ASM
*/
#ifdef	__cplusplus
extern "C" {
#endif
void __cdecl Remove_From_List(void **list, int *index, void * ptr);
void * __cdecl Conquer_Build_Fading_Table(void const *palette, void *dest, int color, int frac);
void __cdecl Fat_Put_Pixel(int x, int y, int color, int size, GraphicViewPortClass &);
void __cdecl strtrim(char *buffer);
long __cdecl Get_EAX( void );
#ifdef	__cplusplus
}
#endif

/*
**	TARCOM.CPP
*/

/*
** TARGET.CPP
*/
COORDINATE  As_Movement_Coord(TARGET target);
AircraftClass *  As_Aircraft(TARGET target);
AnimClass *  As_Animation(TARGET target);
BuildingClass *  As_Building(TARGET target);
BulletClass *  As_Bullet(TARGET target);
CELL  As_Cell(TARGET target);
COORDINATE  As_Coord(TARGET target);
InfantryClass *  As_Infantry(TARGET target);
TeamClass *  As_Team(TARGET target);
TeamTypeClass *  As_TeamType(TARGET target);
TechnoClass *  As_Techno(TARGET target);
//TerrainClass * As_Terrain(TARGET target);
TriggerClass *  As_Trigger(TARGET target);
UnitClass *  As_Unit(TARGET target);
inline bool  Target_Legal(TARGET target) {return(target != TARGET_NONE);};
ObjectClass *  As_Object(TARGET target);

/*
** ULOGIC.CPP
*/
int Terrain_Cost(CELL cell, FacingType facing);

//
DirType Desired_Facing8(int x1, int y1, int x2, int y2);
DirType Desired_Facing256(int srcx, int srcy, int dstx, int dsty);
//

/*
**	Inline miscellaneous functions.
*/
#define	XYP_COORD(x,y)	COORDINATE(((x)*ICON_LEPTON_W)/CELL_PIXEL_W + ((((y)*ICON_LEPTON_H)/CELL_PIXEL_H)<<16))
inline FacingType Dir_Facing(DirType facing) {return (FacingType)(((unsigned char)(facing+0x10)&0xFF)>>5);}
inline DirType Facing_Dir(FacingType facing) {return (DirType)((int)facing << 5);}
inline int Cell_To_Lepton(int cell) {return cell<<8;}
inline int Lepton_To_Cell(int lepton) {return ((unsigned)(lepton + 0x0080))>>8;}
inline CELL XY_Cell(int x, int y) {return ((CELL)(((y)<<6)|(x)));}
inline COORDINATE XY_Coord(int x, int y) {return ((COORDINATE)MAKE_LONG(y, x));}
inline int Coord_X(COORDINATE coord) {return (short)(LOW_WORD(coord));}
inline int Coord_Y(COORDINATE coord) {return (short)(HIGH_WORD(coord));}
inline int Cell_X(CELL cell) {return (int)(((unsigned)cell) & 0x3F);}
inline int Cell_Y(CELL cell) {return (int)(((unsigned)cell) >> 6);}
inline int Dir_Diff(DirType dir1, DirType dir2) {return (int)(*((signed char*)&dir2) - *((signed char*)&dir1));}
inline CELL Coord_XLepton(COORDINATE coord) {return (CELL)(*((unsigned char*)&coord));}
inline CELL Coord_YLepton(COORDINATE coord) {return (CELL)(*(((unsigned char*)&coord)+2));}
//inline COORD CellXY_Coord(unsigned x, unsigned y) {return (COORD)(MAKE_LONG(y<<8, x<<8));}
inline COORDINATE Coord_Add(COORDINATE coord1, COORDINATE coord2) {return (COORDINATE)MAKE_LONG((*((short*)(&coord1)+1) + *((short*)(&coord2)+1)), (*((short*)(&coord1)) + *((short*)(&coord2))));}
inline COORDINATE Coord_Sub(COORDINATE coord1, COORDINATE coord2) {return (COORDINATE)MAKE_LONG((*((short*)(&coord1)+1) - *((short*)(&coord2)+1)), (*((short*)(&coord1)) - *((short*)(&coord2))));}
inline COORDINATE Coord_Snap(COORDINATE coord) {return (COORDINATE)MAKE_LONG((((*(((unsigned short *)&coord)+1))&0xFF00)|0x80), (((*((unsigned short *)&coord))&0xFF00)|0x80));}
inline COORDINATE Coord_Mid(COORDINATE coord1, COORDINATE coord2) {return (COORDINATE)MAKE_LONG((*((unsigned short *)(&coord1)+1) + *((unsigned short *)(&coord2)+1))>>1, (*((unsigned short *)(&coord1)) + *((unsigned short *)(&coord2)))>>1);}
inline COORDINATE Cell_Coord(CELL cell) {return (COORDINATE) MAKE_LONG( (((cell & 0x0FC0)<<2)|0x80), ((((cell & 0x003F)<<1)+1)<<7) );}
inline COORDINATE XYPixel_Coord(int x, int y) {return ((COORDINATE)MAKE_LONG((int)(((long)y*(long)ICON_LEPTON_H)/(long)ICON_PIXEL_H)/*+LEPTON_OFFSET_Y*/, (int)(((long)x*(long)ICON_LEPTON_W)/(long)ICON_PIXEL_W)/*+LEPTON_OFFSET_X*/));}
//inline int Facing_To_16(int facing) {return Facing16[facing];}
inline int Facing_To_32(DirType facing) {return Facing32[facing];}
inline DirType Direction256(COORDINATE coord1, COORDINATE coord2) {return ((DirType)Desired_Facing256(Coord_X(coord1), Coord_Y(coord1), Coord_X(coord2), Coord_Y(coord2)));}
inline DirType Direction(COORDINATE coord1, COORDINATE coord2) {return ((DirType)Desired_Facing256(Coord_X(coord1), Coord_Y(coord1), Coord_X(coord2), Coord_Y(coord2)));}
inline DirType Direction8(COORDINATE coord1, COORDINATE coord2) {return ((DirType)Desired_Facing8(Coord_X(coord1), Coord_Y(coord1), Coord_X(coord2), Coord_Y(coord2)));}
//inline int Direction16(COORDINATE coord1, COORD coord2) {return (Desired_Facing16(Coord_X(coord1), Coord_Y(coord1), Coord_X(coord2), Coord_Y(coord2)));}
inline DirType Direction(CELL cell1, CELL cell2) {return (DirType)(Desired_Facing8(Cell_X(cell1), Cell_Y(cell1), Cell_X(cell2), Cell_Y(cell2)));}
inline COORDINATE Adjacent_Cell(COORDINATE coord, FacingType dir) {return (Coord_Snap(Coord_Add(AdjacentCoord[dir & 0x07], coord)));}
inline COORDINATE Adjacent_Cell(COORDINATE coord, DirType dir) {return Adjacent_Cell(coord, Dir_Facing(dir));}
inline CELL Adjacent_Cell(CELL cell, FacingType dir) {return (CELL)(cell + AdjacentCell[dir]);}
inline CELL Adjacent_Cell(CELL cell, DirType dir) {return (CELL)(cell + AdjacentCell[Dir_Facing(dir)]);}
inline int Lepton_To_Pixel(int lepton) {return ((lepton * ICON_PIXEL_W) + (ICON_LEPTON_W / 2)) / ICON_LEPTON_W;}
inline int Pixel_To_Lepton(int pixel) {return ((pixel * ICON_LEPTON_W) + (ICON_PIXEL_W / 2)) / ICON_PIXEL_W;}
//inline FacingType Facing_To_8(DirType facing) {return (FacingType)(((unsigned char)(facing|0x10))>>5);}
inline COORDINATE XYP_Coord(int x,int y) {return XY_Coord(Pixel_To_Lepton(x), Pixel_To_Lepton(y));};
inline char const * Text_String(int string) {
#ifdef PORTABLE
	// can't find a conquer.eng that contains these
	switch(string)
	{
		case TXT_READING_IMAGE_DATA: return "READING IMAGE DATA";
		case TXT_ANALYZING: return "ANALYZING";
		case TXT_ENHANCING_IMAGE_DATA: return "ENHANCING IMAGE DATA";
		case TXT_ISOLATING_OPERATIONAL_THEATER: return "ISOLATING OPERATIONAL THEATER";
		case TXT_ESTABLISHING_TRADITIONAL_BOUNDARIES: return "ESTABLISHING TRADITIONAL BOUNDARIES";
		case TXT_FOR_VISUAL_REFERENCE: return "FOR VISUAL REFERENCE";
		case TXT_ENHANCING_IMAGE: return "ENHANCING IMAGE";
		case TXT_BONUS_MISSIONS: return "Bonus Missions";
		case TXT_BONUS_MISSION_1: return "Bonus Mission 1";
		case TXT_BONUS_MISSION_2: return "Bonus Mission 2";
		case TXT_BONUS_MISSION_3: return "Bonus Mission 3";
		case TXT_BONUS_MISSION_4: return "Bonus Mission 4";
		case TXT_BONUS_MISSION_5: return "Bonus Mission 5";
	}
#endif
	return(Extract_String(SystemStrings, string));
};


template<class T> inline T Random_Picky(T a, T b, const char *sfile, int line)
{
	sfile = sfile;
	line = line;
	return (T)IRandom((int)a, (int)b);	//, sfile, line);
};

#define Random_Pick(low, high) Random_Picky ( (low), (high), __FILE__, __LINE__)


template<class T> inline T Sim_Random_Pick(T a, T b)
{
	return (T)Sim_IRandom((int)a, (int)b);
};


#ifdef CHEAT_KEYS
#define Check_Ptr(ptr,file,line)  \
{ \
	if (!ptr) { \
		Mono_Clear_Screen(); \
		Mono_Printf("NULL Pointer, Module:%s, line:%d!\n",file,line); \
		Prog_End(); \
		exit(EXIT_SUCCESS); \
	} \
}
#else
#define	Check_Ptr(ptr,file,line)
#endif

/*
** These routines are for coding & decoding multiplayer ID's
*/
inline PlayerColorType MPlayerID_To_ColorIndex(unsigned short id) {return (PlayerColorType)(id >> 4);}
inline HousesType MPlayerID_To_HousesType(unsigned short id) {return ((HousesType)(id & 0x000f)); }
inline unsigned short Build_MPlayerID(int c_idx, HousesType htype) { return ((c_idx << 4) | htype); }




//
//  True if we are the currently in focus windows app
//
extern bool GameInFocus;

extern	int	ScreenWidth;
extern	int	ScreenHeight;
extern "C" void ModeX_Blit (GraphicBufferClass *source);
extern	void Colour_Debug (int call_number);


extern	unsigned char 	*InterpolatedPalettes[100];
extern bool				PalettesRead;
extern unsigned			PaletteCounter;

extern "C"{
	extern unsigned char PaletteInterpolationTable[SIZE_OF_PALETTE][SIZE_OF_PALETTE];
	extern unsigned char *InterpolationPalette;
}

extern void Free_Interpolated_Palettes(void);
extern int Load_Interpolated_Palettes(char const *filename, bool add=false);


#define	CELL_BLIT_ONLY	1
#define	CELL_DRAW_ONLY	2

/***********************************************************************************************
 * Distance -- Determines the lepton distance between two coordinates.                         *
 *                                                                                             *
 *    This routine is used to determine the distance between two coordinates. It uses the      *
 *    Dragon Strike method of distance determination and thus it is very fast.                 *
 *                                                                                             *
 * INPUT:   coord1   -- First coordinate.                                                      *
 *                                                                                             *
 *          coord2   -- Second coordinate.                                                     *
 *                                                                                             *
 * OUTPUT:  Returns the lepton distance between the two coordinates.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int Distance_Coord(COORDINATE coord1, COORDINATE coord2);
#pragma aux Distance_Coord parm [eax] [ebx] \
	modify [edx ebx] \
	value [eax]		= 				\
	"mov	dx,ax"			\
	"sub	dx,bx"			\
	"jg	okx"				\
	"neg	dx"				\
	"okx:"					\
	"shr	eax,16"			\
	"shr	ebx,16"			\
	"sub	ax,bx"			\
	"jg	oky"				\
	"neg	ax"				\
	"oky:"					\
	"cmp	ax,dx"			\
	"jg	ok"				\
	"xchg	ax,dx"			\
	"ok:"						\
	"shr	dx,1"				\
	"add	ax,dx"

inline int Distance(COORDINATE coord1, COORDINATE coord2)
{
#ifdef PORTABLE
	int	diff1, diff2;

	diff1 = Coord_Y(coord1) - Coord_Y(coord2);
	if (diff1 < 0) diff1 = -diff1;
	diff2 = Coord_X(coord1) - Coord_X(coord2);
	if (diff2 < 0) diff2 = -diff2;
	if (diff1 > diff2) {
		return(diff1 + (diff2>>1));
	}
	return(diff2 + (diff1>>1));
#else
	return(Distance_Coord(coord1, coord2));
#endif
}



/***********************************************************************************************
 * Distance -- Determines the cell distance between two cells.                                 *
 *                                                                                             *
 *    Use this routine to determine the distance between the two cells specified. The distance *
 *    is returned in cells.                                                                    *
 *                                                                                             *
 * INPUT:   cell1, cell2   -- The two cells to determine the distance between.                 *
 *                                                                                             *
 * OUTPUT:  Returns with the distance between the two cells in units of cell size.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
inline int Distance(CELL coord1, CELL coord2)
{
	int	diff1, diff2;

	diff1 = Cell_Y(coord1) - Cell_Y(coord2);
	if (diff1 < 0) diff1 = -diff1;
	diff2 = Cell_X(coord1) - Cell_X(coord2);
	if (diff2 < 0) diff2 = -diff2;
	if (diff1 > diff2) {
		return(diff1 + (diff2>>1));
	}
	return(diff2 + (diff1>>1));
}


/***********************************************************************************************
 * CellClass::Cell_Number -- Returns the cell ID number for this cell object.                  *
 *                                                                                             *
 *    Call this routine if you wish to determine what the cell number ID is for the currrent   *
 *    cell object. This ID number is the index number into the cell array.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the cell number for this cell object.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
inline CELL CellClass::Cell_Number(void) const
{
	return(Map.ID(this));
}
#if(0)
#ifndef NOMEMCHECK
#define NO_INTERCEPT
#include	"memcheck.h"
#endif
#endif

void WWDOS_Shutdown(void);

#endif