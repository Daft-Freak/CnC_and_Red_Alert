add_library(jshell STATIC
	rotbmp.cpp
	sprite.cpp
)
target_link_libraries(jshell port)

add_library(tech STATIC
    lzo1x_c.cpp
	lzo1x_d.cpp
	lzopipe.cpp
	lzostraw.cpp
	lzw.cpp
	lzwpipe.cpp
	lzwstraw.cpp
	buff.cpp
	bench.cpp
	rect.cpp
	mpu.cpp
	readline.cpp
	pkpipe.cpp
	pkstraw.cpp
	xstraw.cpp
	xpipe.cpp
	rndstraw.cpp
	pk.cpp
	blwstraw.cpp
	straw.cpp
	b64straw.cpp
	lcwstraw.cpp
	crcstraw.cpp
	shastraw.cpp
	ramfile.cpp
	ini.cpp
	lcwpipe.cpp
	#lcwcomp.asm
	lcw.cpp
	crcpipe.cpp
	shapipe.cpp
	pipe.cpp
	blowpipe.cpp
	b64pipe.cpp
	base64.cpp
	fixed.cpp
	mixfile.cpp
	cdfile.cpp
	bfiofile.cpp
	rawfile.cpp
	mp.cpp
	int.cpp
	monoc.cpp
	random.cpp
	rgb.cpp
	hsv.cpp
	palette.cpp
	blowfish.cpp
	sha.cpp
	crc.cpp
	sendfile.cpp
	lcwuncmp.cpp
)
target_link_libraries(tech port vqa32)
target_compile_definitions(tech PUBLIC WINSOCK_IPX=1)

set(RA_SOURCES
	aadata.cpp
	abstract.cpp
	adata.cpp
	aircraft.cpp
	anim.cpp
	audio.cpp
	bar.cpp
	base.cpp
	bbdata.cpp
	bdata.cpp
	building.cpp
	bullet.cpp
	cargo.cpp
	carry.cpp
	ccfile.cpp
	ccini.cpp
	ccptr.cpp
	cdata.cpp
	cell.cpp
	checkbox.cpp
	cheklist.cpp
	colrlist.cpp
	combat.cpp
	combuf.cpp
	connect.cpp
	conquer.cpp
	const.cpp
	control.cpp
	coord.cpp
	crate.cpp
	credits.cpp
	crew.cpp
	debug.cpp
	dial8.cpp
	dialog.cpp
	display.cpp
	door.cpp
	drive.cpp
	drop.cpp
	dynavec.cpp
	edit.cpp
	egos.cpp
	ending.cpp
	event.cpp
	expand.cpp
	face.cpp
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
	intro.cpp
	iomap.cpp
	ioobj.cpp
	ipx.cpp
	ipxaddr.cpp
	ipxconn.cpp
	ipxgconn.cpp
	ipxmgr.cpp
	# ipxprot.asm
	jshell.cpp
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
	rules.cpp
	saveload.cpp
	scenario.cpp
	score.cpp
	scroll.cpp
	sdata.cpp
	session.cpp
	shapebtn.cpp
	sidebar.cpp
	slider.cpp
	smudge.cpp
	sounddlg.cpp
	special.cpp
	startup.cpp
	statbtn.cpp
	super.cpp
	tab.cpp
	taction.cpp
	target.cpp
	tdata.cpp
	team.cpp
	teamtype.cpp
	techno.cpp
	template.cpp
	terrain.cpp
	tevent.cpp
	textbtn.cpp
	theme.cpp
	toggle.cpp
	tracker.cpp
	trigger.cpp
	trigtype.cpp
	txtlabel.cpp
	udata.cpp
	unit.cpp
	utracker.cpp
	vdata.cpp
	vector.cpp
	version.cpp
	vessel.cpp
	visudlg.cpp
	vortex.cpp
	warhead.cpp
	weapon.cpp
	mcimovie.cpp
	mci.cpp
	mpgset.cpp
	iconlist.cpp
	wol_main.cpp
	wol_chat.cpp
	rawolapi.cpp
	wolapiob.cpp
	# cominit.cpp # COM
	wol_logn.cpp
	woledit.cpp
	# passedit.obj ?
	seditdlg.cpp
	#dibfile.cpp #very win32-api
	#dibutil.cpp #
	wol_gsup.cpp
	udpaddr.cpp
	wsproto.cpp
	wspudp.cpp
	wspipx.cpp
	_wsproto.cpp
	#tooltip.cpp # requires WOLAPI_INTEGRATION
	wol_opt.cpp
	wol_cgam.cpp
	bigcheck.cpp
	wol_dnld.cpp
	wolstrng.cpp
	# win32
	#2keyfbuf.asm
	#cpuid.asm
	#getcpu.cpp # unused
	interpal.cpp
	#winasm.asm
	#2txtprnt.asm
	writepcx.cpp
	2keyfram.cpp
	tcpip.cpp
	internet.cpp
	#dde.cpp # DDE
	#ccdde.cpp # DDE
	stats.cpp
	packet.cpp
	field.cpp

	# new
	nondosstub.cpp
	2keyfbuf.cpp
	winasm.cpp
)

if(PICO_SDK_PATH)
	target_link_libraries(jshell picolib)
	target_link_libraries(tech picolib)

	add_executable(raspberry-alert ${RA_SOURCES} picostub.cpp)

	target_compile_definitions(raspberry-alert PRIVATE
		ENGLISH=1
		PORTABLE=1 # to indicate not windows or dos sometimes
		LORES=1
	)
	target_link_libraries(raspberry-alert PRIVATE tech jshell port picolib vqa32)
	
	set_target_properties(raspberry-alert PROPERTIES PICO_TARGET_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/../PICOLIB/memmap.ld)
	pico_add_link_depend(raspberry-alert ${CMAKE_CURRENT_LIST_DIR}/../PICOLIB/memmap.ld)

	pico_enable_stdio_uart(raspberry-alert 1)
	pico_add_extra_outputs(raspberry-alert)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/raspberry-alert.uf2
		DESTINATION bin
	)
elseif(ESP_TARGET)
	target_link_libraries(jshell esplib)
	target_link_libraries(tech esplib)

	add_executable(esp-ra ${RA_SOURCES} picostub.cpp)

	target_compile_definitions(esp-ra PRIVATE
		ENGLISH=1
		PORTABLE=1 # to indicate not windows or dos sometimes
		LORES=1
	)
	target_link_libraries(esp-ra PRIVATE tech jshell port esplib vqa32)

	idf_build_executable(esp-ra)
else()
	target_link_libraries(jshell sdllib)
	target_link_libraries(tech sdllib)

	add_executable(rasdl WIN32 ${RA_SOURCES} sdlstub.cpp)

	if(WIN32)
		target_sources(rasdl PRIVATE
			dde.cpp
			ccdde.cpp
			cc_icon.rc
		)
		target_link_libraries(rasdl PRIVATE wsock32)
	endif()

	target_compile_definitions(rasdl PRIVATE
		ENGLISH=1  # TODO: also french/german
		PORTABLE=1 # to indicate not windows or dos sometimes
	)
	target_link_libraries(rasdl PRIVATE tech jshell port sdllib vqa32) # mpgdll wwipx32

	install(TARGETS rasdl)

	if(EMSCRIPTEN)
		set_target_properties(rasdl PROPERTIES
			SUFFIX ".html"
			LINK_FLAGS "-sENVIRONMENT=web -sASYNCIFY -sASYNCIFY_STACK_SIZE=8192 -sFORCE_FILESYSTEM -sINITIAL_MEMORY=32mb"
		)
		# this does not include the files, that would require something like
		# file_packager files.data --js-output=files.js --preload *.mix REDALERT.INI
		# (which is huge if you include the movies)
		# and adding files.js to the shell html
	endif()
endif()