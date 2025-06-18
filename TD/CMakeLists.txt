add_executable(tdsdl WIN32
    super.cpp
	aadata.cpp
	#winstub.cpp
	#winasm.asm
	abstract.cpp
	adata.cpp
	aircraft.cpp
	anim.cpp
	audio.cpp
	base.cpp
	bbdata.cpp
	bdata.cpp
	building.cpp
	bullet.cpp
	cargo.cpp
	ccfile.cpp
	cdata.cpp
	cdfile.cpp
	cell.cpp
	checkbox.cpp
	cheklist.cpp
	colrlist.cpp
	combat.cpp
	combuf.cpp
	confdlg.cpp
	connect.cpp
	conquer.cpp
	const.cpp
	control.cpp
	coord.cpp
	credits.cpp
	crew.cpp
	debug.cpp
	dial8.cpp
	dialog.cpp
	display.cpp
	door.cpp
	drive.cpp
	edit.cpp
	event.cpp
	ending.cpp
	expand.cpp
	facing.cpp
	factory.cpp
	findpath.cpp
	flasher.cpp
	fly.cpp
	foot.cpp
	fuse.cpp
	gadget.cpp
	gamedlg.cpp
	gauge.cpp
	globals.cpp
	goptions.cpp
	gscreen.cpp
	hdata.cpp
	heap.cpp
	help.cpp
	house.cpp
	idata.cpp
	infantry.cpp
	ini.cpp
	init.cpp
	internet.cpp
	interpal.cpp
	intro.cpp
	iomap.cpp
	ioobj.cpp
	ipx.cpp
	ipxaddr.cpp
	ipxconn.cpp
	ipxgconn.cpp
	ipxmgr.cpp
	ipx95.cpp
	jshell.cpp
	#keyfbuff.asm
	keyframe.cpp
	layer.cpp
	link.cpp
	list.cpp
	loaddlg.cpp
	logic.cpp
	map.cpp
	mapeddlg.cpp
	mapedit.cpp
	mapedplc.cpp
	mapedtm.cpp
	mapsel.cpp
	menus.cpp
	mission.cpp
	mixfile.cpp
	mouse.cpp
	mplayer.cpp
	msgbox.cpp
	msglist.cpp
	netdlg.cpp
	nullconn.cpp
	nulldlg.cpp
	nullmgr.cpp
	object.cpp
	odata.cpp
	options.cpp
	overlay.cpp
	power.cpp
	profile.cpp
	queue.cpp
	radar.cpp
	radio.cpp
	reinf.cpp
	saveload.cpp
	scenario.cpp
	score.cpp
	scroll.cpp
	sdata.cpp
	shapebtn.cpp
	sidebar.cpp
	slider.cpp
	smudge.cpp
	sounddlg.cpp
	special.cpp
	startup.cpp
	#support.asm
	tab.cpp
	tarcom.cpp
	target.cpp
	tcpip.cpp
	tdata.cpp
	team.cpp
	teamtype.cpp
	techno.cpp
	template.cpp
	terrain.cpp
	textbtn.cpp
	theme.cpp
	toggle.cpp
	trigger.cpp
	turret.cpp
	txtlabel.cpp
	#txtprnt.asm
	udata.cpp
	unit.cpp
	vector.cpp
	visudlg.cpp
	stats.cpp
    monoc.cpp
    rawfile.cpp

    # these files are identical (or close enough) to RA
    noseqcon.cpp
    ../RA/field.cpp
    ../RA/packet.cpp
    rand.cpp
    utracker.cpp

    # this one was copied from RA
    face.cpp

    # also need these
    ../RA/lcwuncmp.cpp
    ../RA/2keyfbuf.cpp
	../RA/winasm.cpp

    ipxstub.cpp
    nondosstub.cpp
    palette.cpp
    sdlstub.cpp
    support.cpp
)

if(WIN32)
	target_sources(tdsdl PRIVATE
		../RA/dde.cpp
		ccdde.cpp
		cc_icon.rc
	)
	target_link_libraries(tdsdl PRIVATE wsock32)
endif()

target_compile_definitions(tdsdl PRIVATE
	PORTABLE=1 # to indicate not windows or dos sometimes
    TD=1
)
target_include_directories(tdsdl PRIVATE ../RA) # should probably structure this better
target_link_libraries(tdsdl PRIVATE port sdllib vqa32)

install(TARGETS tdsdl)

if(EMSCRIPTEN)
	set_target_properties(tdsdl PROPERTIES
		SUFFIX ".html"
		LINK_FLAGS "-sENVIRONMENT=web -sASYNCIFY -sASYNCIFY_STACK_SIZE=8192 -sFORCE_FILESYSTEM -sINITIAL_MEMORY=128mb"
	)
	# this does not include the files, that would require something like
	# file_packager files.data --js-output=files.js --preload *.mix
	# (which is huge if you include the movies)
	# and adding files.js to the shell html
endif()