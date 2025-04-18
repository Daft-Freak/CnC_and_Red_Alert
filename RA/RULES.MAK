#
#	Command & Conquer Red Alert(tm)
#	Copyright 2025 Electronic Arts Inc.
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Size of the program shape buffer (passed to MAKESHPS.EXE)
SHAPEBUFFSIZE = 20000

# Default paths to use
ROOTCD = v:\projects\c&c0\cstrike2\cd\\
ROOTDIR = ..\\
MAPDIR  = ..\\
ROOTCODEDIR = .\\
ABSROOT = ..\\
WAVDIR  = f:\projects\c&c0\audio\amingame\\
SCOREDIR = f:\projects\c&c0\audio\amingame\scores\\
VQADIR  = f:\projects\c&c0\art\movies\\
LANGDIR =
INGAMEDIR = ingame
TEMPDIR = s:


# Override paths
!if $d(GERMAN)
ROOTCD = v:\projects\c&c0\cstrike2\cdger\\
ROOTDIR = ..\\
MAPDIR  = ..\ger
ROOTCODEDIR = .\\
ABSROOT = ..\\
WAVDIR  = f:\projects\c&c0\audio\amingame\\
LANGDIR = \german
INGAMEDIR = ingamegr
#VQADIR = s:t\\
VQADIR = t:movies\\
TEMPDIR = s:
!endif

!if $d(FRENCH)
ROOTCD = v:\projects\c&c0\cstrike2\cdfre\\
ROOTDIR = ..\\
MAPDIR  = ..\fre
ROOTCODEDIR = .\\
ABSROOT = ..\\
WAVDIR  = f:\projects\c&c0\audio\amingame\
LANGDIR = \french
INGAMEDIR=ingamefr
VQADIR  = t:movies\\
TEMPDIR = s:
!endif


# Directory where temporary mixfiles can be stored.
!if $d(TEMP_MIX_DIR)
TEMP_MIX_DIR = $(TEMP_MIX_DIR)
!else
TEMP_MIX_DIR = $(TEMPDIR)t\m
!endif

!if $d(CPS)
TEMP_CPS_DIR = $(CPS)
!else
TEMP_CPS_DIR = $(TEMPDIR)t
!endif


#.path.a6a = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a6j = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a8  = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a80 = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a81 = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a82 = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a83 = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a84 = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a8a = $(TEMP_CPS_DIR)\\            # Game formatted audio data
#.path.a8j = $(TEMP_CPS_DIR)\\            # Game formatted audio data
.path.a6  = $(TEMP_CPS_DIR)\\            # Game formatted audio data
.path.anm = $(ROOTDIR)art\$(INGAMEDIR)\\    # Raw animation file
.path.ash = .\\                          # Autogenerated assembly header file
.path.asm = .\\                          # Raw assembly file
.path.aud = $(TEMP_CPS_DIR)\\            # Game formatted audio data
.path.c   = .\\                          # Raw C file
.path.cd1 = $(ROOTCD)CD4\\               # CD #1 (Allies)
.path.cd2 = $(ROOTCD)CD2\\               # CD #1 (Soviet)
.path.cpp = .\\                          # Raw C++ file
.path.cps = $(TEMP_CPS_DIR)\\            # Compressed art
.path.dat = $(TEMP_CPS_DIR)\\            # Generic data
.path.des = $(TEMP_CPS_DIR)\\            # Desert icons
.path.dip = $(TEMP_CPS_DIR)\\            # Generic compressed text (obsolete)
.path.eng = $(TEMP_CPS_DIR)\\            # English compressed text
.path.exe = $(ROOTDIR)run\\              # Executable (relative path from OBJ directory)
.path.fin = $(ROOTDIR)art\ingame\fin\\   # Finale source animation
.path.fnt = $(TEMP_CPS_DIR)\\            # Font (compressed)
.path.foc = $(ROOTDIR)audio\french\\     # French digitized voices
.path.fre = $(TEMP_CPS_DIR)\\            # French compressed text
.path.gdi = $(TEMP_CPS_DIR)\\            # Good guy compressed files
.path.ger = $(TEMP_CPS_DIR)\\            # German compressed text
.path.goc = $(ROOTDIR)audio\german\\     # German digitized voices
.path.h   = .\\                          # C header file
.path.hi  = $(TEMP_CPS_DIR)\\            # Shape file (processed)
.path.hnt = $(TEMP_CPS_DIR)\\            # Font (compressed)
.path.i   = .\\                          # Assembly header
.path.icn = $(TEMP_CPS_DIR)\\            # Icon file (compressed)
.path.ini = $(MAPDIR)maps\\              # INI control files
.path.int = $(TEMP_CPS_DIR)\\            # Interior icons
.path.j6  = $(TEMP_CPS_DIR)\\            # Game formatted audio data
.path.jp  = $(TEMP_CPS_DIR)\\            # Dinosaur compressed files
.path.jun = $(TEMP_CPS_DIR)\\            # Jungle icons
.path.juv = $(TEMP_CPS_DIR)\\            # Juvenile sound effects.
.path.lbm = $(ROOTDIR)art\ingame\\ # Source art (uncompressed)
.path.lnt = $(TEMP_CPS_DIR)\\            # Font (compressed)
.path.low = $(TEMP_CPS_DIR)\\            # Shape file (processed)
.path.lut = $(TEMP_CPS_DIR)\\            # Look up tables
.path.mak = .\\                          # Shape making control file
.path.map = $(TEMP_CPS_DIR)\\            # Icon map data file
.path.mid = $(ROOTDIR)audio\mid\\        # Midi file
.path.mix = $(TEMP_MIX_DIR)\\            # Mix-file location.
.path.mpr = $(MAPDIR)maps\\              # Multiplayer scenario files
.path.mrf = $(TEMP_CPS_DIR)\\            # Palette morph data files
.path.nod = $(TEMP_CPS_DIR)\\            # Bad guy compressed files
.path.o   = obj\\                        # Overlay object file
.path.obj = obj\\                        # Object file
.path.out = .\\                          # Temporary parser file.
.path.pak = $(ROOTDIR)run\\              # Packed file
.path.pal = $(TEMP_CPS_DIR)\\            # Palette file (processed)
.path.pcx = $(TEMP_CPS_DIR)\\            #.pcx files
.path.pkt = $(MAPDIR)maps\\              # Scenario grouping control files
.path.r00 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.r01 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.r02 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.r03 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.shp = $(TEMP_CPS_DIR)\\            # Shape file (processed)
.path.sno = $(TEMP_CPS_DIR)\\            # Snow icons
.path.sym = .\\                          # Precompiled header symbol file.
.path.tbl = $(TEMP_CPS_DIR)\\            # Remap table file.
.path.tem = $(TEMP_CPS_DIR)\\            # Temperate icons
.path.txt = $(ROOTDIR)code\eng\\         # Default location for text files.
.path.v00 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.v01 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.v02 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.v03 = $(TEMP_CPS_DIR)\\            # Infantry response.
.path.v16 = $(TEMP_CPS_DIR)\\            # Game formatted audio data
.path.vqa = $(VQADIR)vq$(LANGDIR)\\      # VQA movie files location.
.path.vqp = $(VQADIR)vq$(LANGDIR)\\      # VQA Movie Interpolated palette files
!if $d(FRENCH)
.path.vqa = $(VQADIR)vq\french\\      # VQA movie files location.
.path.vqp = $(VQADIR)vq\french\\      # VQA Movie Interpolated palette files
!endif
.path.wav = $(WAVDIR)                    # Digitized sample file (generic)
.path.win = $(TEMP_CPS_DIR)\\            # Winter icons
.path.wsa = $(TEMP_CPS_DIR)\\            # Processed animation file
.path.wv  = $(ROOTDIR)audio\wv\\         # Digitized sample file (generic)
.path.xmi = $(ROOTDIR)audio\xmi\\        # MT-32 midi scores


##########################################################################
# Rules

#
# Rule for converting anim files.
#
.anm.wsa:
	utils\animate $*.anm $(.path.anm)$&.lbm $(.path.wsa)$&.wsa -l -f -p

#
# Rule for converting art files.
#
.lbm.cps:
	utils\wwcomp $*.lbm $(.path.cps)$&.cps -ex -i

#
# Rule for converting outtake first frame art files.
#
{$(.path.lbm)outtake}.lbm.cps:
	utils\wwcomp $*.lbm $(.path.cps)$&.cps -ex

{$(.path.anm)outtake}.anm.wsa:
	utils\animate $*.anm $(.path.anm)outtake\$&.lbm $(.path.wsa)$&.wsa -l -f -p

{$(.path.anm)score}.anm.wsa:
	utils\animate $*.anm $(.path.wsa)$&.wsa -p

#########################################################
# Rule for creating palette files.
#
{$(.path.lbm)palettes}.lbm.pal:
	utils\wwcomp $*.lbm -ppal
	copy $&.pal $(.path.pal)$&.pal
	del $&.pal

{$(.path.lbm)score}.lbm.pal:
	utils\wwcomp $*.lbm -ppal
	copy $&.pal $(.path.pal)$&.pal
	del $&.pal

#########################################################
# Rule for creating vehicles.
{$(.path.anm)units}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

#########################################################
# Rule for creating buildings.
{$(.path.anm)building}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

#########################################################
# Rule for creating misc shape animations.
{$(.path.anm)anim}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

#########################################################
# Rule for converting font files.
{$(.path.lbm)fonts}.lbm.fnt:
	echo Creating font file "$&.fnt".
	utils\fontmake $*.lbm -o$(.path.fnt)$&.fnt

{$(.path.lbm)hires}.lbm.hnt:
	echo Creating hi-res font file "$&.fnt".
	utils\fontmake $*.lbm -o$(.path.hnt)$&.hnt

{$(.path.lbm)hires}.lbm.fnt:
	echo Creating lo-res font file "$&.fnt".
	utils\fontmake $*.lbm -o$(.path.hnt)$&.fnt

{$(.path.lbm)lores}.lbm.lnt:
	echo Creating lo-res font file "$&.fnt".
	utils\fontmake $*.lbm -o$(.path.lnt)$&.lnt

{$(.path.lbm)lores}.lbm.fnt:
	echo Creating lo-res font file "$&.fnt".
	utils\fontmake $*.lbm -o$(.path.lnt)$&.fnt

#########################################################
# Rules for creating overlay files.
{$(.path.lbm)overlay}.lbm.cps:
	utils\wwcomp $*.lbm $(.path.cps)$&.cps

{$(.path.anm)overlay}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

#########################################################
# Rules for creating temperate files.
{$(.path.lbm)temperat}.lbm.tem:
	utils\iconmap -r -w3 -o$(.path.tem)$&.tem $*.lbm

{$(.path.anm)temperat}.anm.tem:
	utils\keyframe $*.anm $(.path.tem)$&.tem -l

#########################################################
# Rules for creating interior files.
{$(.path.lbm)interior}.lbm.int:
	utils\iconmap -r -w3 -o$(.path.int)$&.int $*.lbm

{$(.path.anm)interior}.anm.int:
	utils\keyframe $*.anm $(.path.int)$&.int -l

#########################################################
# Rules for creating winter files.
{$(.path.lbm)snow}.lbm.sno:
	utils\iconmap -r -w3 -o$(.path.sno)$&.sno $*.lbm

{$(.path.anm)snow}.anm.sno:
	utils\keyframe $*.anm $(.path.sno)$&.sno -l

#########################################################
# Rules for creating desert files.
{$(.path.lbm)desert}.lbm.des:
	utils\iconmap -r -w3 -o$(.path.des)$&.des $*.lbm

{$(.path.anm)desert}.anm.des:
	utils\keyframe $*.anm $(.path.des)$&.des -l

#########################################################
# Generic icon file creation.
.lbm.icn:
	utils\iconmap -r -w3 -o$(.path.icn)$&.icn $*.lbm


#########################################################
# Text files.
{eng}.txt.eng:
	utils\textmake $*.txt $(.path.eng)$&.eng $&.h

{ger}.txt.ger:
	utils\textmake $*.txt $(.path.ger)$&.ger $&.h

{fre}.txt.fre:
	utils\textmake $*.txt $(.path.fre)$&.fre $&.h

#########################################################
# Score/mapsel files.
{$(.path.anm)score}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

{$(.path.lbm)score}.lbm.cps:
	utils\wwcomp $*.lbm $(.path.cps)$&.cps -ex -i


#########################################################
# Generic shape files.
#.lbm.shp:
#       utils\makeshps -q $*.lbm $&.mak $(.path.cps)$&.shp $(SHAPEBUFFSIZE)

{$(.path.anm)generic}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

.c.lob:
	utils\LINT -ml -si2 -sl4 +v $(WWLIB)WWLIB.LNT $*.c >$&.err

# Generic shape file creation.
{$(.path.anm)}.anm.shp:
	utils\keyframe $*.anm $(.path.shp)$&.shp -l

{$(.path.lbm)units}.lbm.cps:
	utils\wwcomp $*.lbm $(.path.cps)$&.cps -ex -i


##########################################################
# Hi-res data files
#
{$(.path.anm)hires}.anm.hi:
	utils\keyframe $*.anm $(.path.hi)$&.hi -l

# Hi-res font files
{$(.path.lbm)hires}.lbm.fnt:
	echo Creating font file "$&.fnt".
	utils\fontmake $*.lbm -o$(.path.fnt)$&.fnt

#########################################################
# Rule for creating low res data files.
{$(.path.anm)lores}.anm.low:
	utils\keyframe $*.anm $(.path.low)$&.low -l

{$(.path.lbm)lores}.lbm.cps:
	utils\wwcomp $*.lbm $(.path.cps)$&.cps -ex -i

#######################################
# Rules to convert 16 bit audio files.
{$(SCOREDIR)}.wav.aud:
	utils\audiomak $(SCOREDIR)$&.wav $(.path.aud)$&.aud

{$(.path.wav)speech$(LANGDIR)}.wav.aud:
	utils\audiomak $(.path.wav)speech$(LANGDIR)\$&.wav $(.path.aud)$&.aud

{$(.path.wav)sfx\tanya$(LANGDIR)}.wav.aud:
	utils\audiomak $(.path.wav)sfx\tanya$(LANGDIR)\$&.wav $(.path.aud)$&.aud

{$(.path.wav)sfx$(LANGDIR)}.wav.aud:
	utils\audiomak $(.path.wav)sfx$(LANGDIR)\$&.wav $(.path.aud)$&.aud

{$(.path.wav)sfx\v00$(LANGDIR)}.wav.v00:
	utils\audiomak $(.path.wav)sfx\v00$(LANGDIR)\$&.wav $(.path.aud)$&.v00

{$(.path.wav)sfx\v01$(LANGDIR)}.wav.v01:
	utils\audiomak $(.path.wav)sfx\v01$(LANGDIR)\$&.wav $(.path.aud)$&.v01

{$(.path.wav)sfx\v02$(LANGDIR)}.wav.v02:
	utils\audiomak $(.path.wav)sfx\v02$(LANGDIR)\$&.wav $(.path.aud)$&.v02

{$(.path.wav)sfx\v03$(LANGDIR)}.wav.v03:
	utils\audiomak $(.path.wav)sfx\v03$(LANGDIR)\$&.wav $(.path.aud)$&.v03

{$(.path.wav)sfx\r00$(LANGDIR)}.wav.r00:
	utils\audiomak $(.path.wav)sfx\r00$(LANGDIR)\$&.wav $(.path.aud)$&.r00

{$(.path.wav)sfx\r01$(LANGDIR)}.wav.r01:
	utils\audiomak $(.path.wav)sfx\r01$(LANGDIR)\$&.wav $(.path.aud)$&.r01

{$(.path.wav)sfx\r02$(LANGDIR)}.wav.r02:
	utils\audiomak $(.path.wav)sfx\r02$(LANGDIR)\$&.wav $(.path.aud)$&.r02

{$(.path.wav)sfx\r03$(LANGDIR)}.wav.r03:
	utils\audiomak $(.path.wav)sfx\r03$(LANGDIR)\$&.wav $(.path.aud)$&.r03

