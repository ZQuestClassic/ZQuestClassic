if (WANT_VENDORED_DUMB)
	add_library(dumb STATIC
		dumb/src/core/atexit.c
		dumb/src/core/duhlen.c
		dumb/src/core/duhtag.c
		dumb/src/core/dumbfile.c
		dumb/src/core/loadduh.c
		dumb/src/core/makeduh.c
		dumb/src/core/rawsig.c
		dumb/src/core/readduh.c
		dumb/src/core/register.c
		dumb/src/core/rendduh.c
		dumb/src/core/rendsig.c
		dumb/src/core/unload.c
		dumb/src/helpers/barray.c
		dumb/src/helpers/clickrem.c
		dumb/src/helpers/lpc.c
		dumb/src/helpers/memfile.c
		dumb/src/helpers/resamp2.inc
		dumb/src/helpers/resamp3.inc
		dumb/src/helpers/resample.c
		dumb/src/helpers/resample.inc
		dumb/src/helpers/resampler.c
		dumb/src/helpers/riff.c
		dumb/src/helpers/sampbuf.c
		dumb/src/helpers/silence.c
		dumb/src/helpers/stdfile.c
		dumb/src/helpers/tarray.c
		dumb/src/it/itload.c
		dumb/src/it/itload2.c
		dumb/src/it/itmisc.c
		dumb/src/it/itorder.c
		dumb/src/it/itread.c
		dumb/src/it/itread2.c
		dumb/src/it/itrender.c
		dumb/src/it/itunload.c
		dumb/src/it/load669.c
		dumb/src/it/load6692.c
		dumb/src/it/loadamf.c
		dumb/src/it/loadamf2.c
		dumb/src/it/loadany.c
		dumb/src/it/loadany2.c
		dumb/src/it/loadasy.c
		dumb/src/it/loadasy2.c
		dumb/src/it/loadmod.c
		dumb/src/it/loadmod2.c
		dumb/src/it/loadmtm.c
		dumb/src/it/loadmtm2.c
		dumb/src/it/loadokt.c
		dumb/src/it/loadokt2.c
		dumb/src/it/loadoldpsm.c
		dumb/src/it/loadoldpsm2.c
		dumb/src/it/loadpsm.c
		dumb/src/it/loadpsm2.c
		dumb/src/it/loadptm.c
		dumb/src/it/loadptm2.c
		dumb/src/it/loadriff.c
		dumb/src/it/loadriff2.c
		dumb/src/it/loads3m.c
		dumb/src/it/loads3m2.c
		dumb/src/it/loadstm.c
		dumb/src/it/loadstm2.c
		dumb/src/it/loadxm.c
		dumb/src/it/loadxm2.c
		dumb/src/it/ptmeffect.c
		dumb/src/it/read669.c
		dumb/src/it/read6692.c
		dumb/src/it/readam.c
		dumb/src/it/readamf.c
		dumb/src/it/readamf2.c
		dumb/src/it/readany.c
		dumb/src/it/readany2.c
		dumb/src/it/readasy.c
		dumb/src/it/readdsmf.c
		dumb/src/it/readmod.c
		dumb/src/it/readmod2.c
		dumb/src/it/readmtm.c
		dumb/src/it/readokt.c
		dumb/src/it/readokt2.c
		dumb/src/it/readoldpsm.c
		dumb/src/it/readpsm.c
		dumb/src/it/readptm.c
		dumb/src/it/readriff.c
		dumb/src/it/reads3m.c
		dumb/src/it/reads3m2.c
		dumb/src/it/readstm.c
		dumb/src/it/readstm2.c
		dumb/src/it/readxm.c
		dumb/src/it/readxm2.c
		dumb/src/it/xmeffect.c
	)
	target_include_directories(dumb PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dumb/include>
		$<INSTALL_INTERFACE:include/dumb>
	)
	install(TARGETS dumb EXPORT AllegroTargets)
	set_target_properties(dumb PROPERTIES POSITION_INDEPENDENT_CODE ON)
	set_target_properties(dumb PROPERTIES FOLDER Sound)

	set(DUMB_FOUND TRUE PARENT_SCOPE)
	set(DUMB_LIBRARIES dumb PARENT_SCOPE)
	set(DUMB_LIBRARY dumb PARENT_SCOPE)
	set(DUMB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dumb/include" PARENT_SCOPE)
	set(DUMB_INCLUDE_DIRS "${DUMB_INCLUDE_DIR}" PARENT_SCOPE)
	set(SUPPORT_DUMB 1 CACHE INTERNAL "Force dumb support in Allegro")
endif()
