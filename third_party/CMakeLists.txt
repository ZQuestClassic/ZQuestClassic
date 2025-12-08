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

if(WANT_VENDORED_LIBJPEG_TURBO)
	add_library(libjpeg_turbo STATIC
		libjpeg-turbo/src/jaricom.c
		libjpeg-turbo/src/jcapimin.c
		libjpeg-turbo/src/jcarith.c
		libjpeg-turbo/src/jchuff.c
		libjpeg-turbo/src/jcicc.c
		libjpeg-turbo/src/jcinit.c
		libjpeg-turbo/src/jcmarker.c
		libjpeg-turbo/src/jcmaster.c
		libjpeg-turbo/src/jcomapi.c
		libjpeg-turbo/src/jcparam.c
		libjpeg-turbo/src/jcphuff.c
		libjpeg-turbo/src/jctrans.c
		libjpeg-turbo/src/jdapimin.c
		libjpeg-turbo/src/jdarith.c
		libjpeg-turbo/src/jdatadst-tj.c
		libjpeg-turbo/src/jdatadst.c
		libjpeg-turbo/src/jdatasrc-tj.c
		libjpeg-turbo/src/jdatasrc.c
		libjpeg-turbo/src/jdhuff.c
		libjpeg-turbo/src/jdicc.c
		libjpeg-turbo/src/jdinput.c
		libjpeg-turbo/src/jdmarker.c
		libjpeg-turbo/src/jdmaster.c
		libjpeg-turbo/src/jdphuff.c
		libjpeg-turbo/src/jdtrans.c
		libjpeg-turbo/src/jerror.c
		libjpeg-turbo/src/jfdctflt.c
		libjpeg-turbo/src/jmemmgr.c
		libjpeg-turbo/src/jmemnobs.c
		libjpeg-turbo/src/jpeg_nbits.c
		libjpeg-turbo/src/transupp.c
		libjpeg-turbo/src/turbojpeg.c

		# 8 and 12 bit wrappers.
		libjpeg-turbo/src/wrapper/jcapistd-12.c
		libjpeg-turbo/src/wrapper/jcapistd-8.c
		libjpeg-turbo/src/wrapper/jccoefct-12.c
		libjpeg-turbo/src/wrapper/jccoefct-8.c
		libjpeg-turbo/src/wrapper/jccolor-12.c
		libjpeg-turbo/src/wrapper/jccolor-8.c
		libjpeg-turbo/src/wrapper/jcdctmgr-12.c
		libjpeg-turbo/src/wrapper/jcdctmgr-8.c
		libjpeg-turbo/src/wrapper/jcmainct-12.c
		libjpeg-turbo/src/wrapper/jcmainct-8.c
		libjpeg-turbo/src/wrapper/jcprepct-12.c
		libjpeg-turbo/src/wrapper/jcprepct-8.c
		libjpeg-turbo/src/wrapper/jcsample-12.c
		libjpeg-turbo/src/wrapper/jcsample-8.c
		libjpeg-turbo/src/wrapper/jdapistd-12.c
		libjpeg-turbo/src/wrapper/jdapistd-8.c
		libjpeg-turbo/src/wrapper/jdcoefct-12.c
		libjpeg-turbo/src/wrapper/jdcoefct-8.c
		libjpeg-turbo/src/wrapper/jdcolor-12.c
		libjpeg-turbo/src/wrapper/jdcolor-8.c
		libjpeg-turbo/src/wrapper/jddctmgr-12.c
		libjpeg-turbo/src/wrapper/jddctmgr-8.c
		libjpeg-turbo/src/wrapper/jdmainct-12.c
		libjpeg-turbo/src/wrapper/jdmainct-8.c
		libjpeg-turbo/src/wrapper/jdmerge-12.c
		libjpeg-turbo/src/wrapper/jdmerge-8.c
		libjpeg-turbo/src/wrapper/jdpostct-12.c
		libjpeg-turbo/src/wrapper/jdpostct-8.c
		libjpeg-turbo/src/wrapper/jdsample-12.c
		libjpeg-turbo/src/wrapper/jdsample-8.c
		libjpeg-turbo/src/wrapper/jfdctfst-12.c
		libjpeg-turbo/src/wrapper/jfdctfst-8.c
		libjpeg-turbo/src/wrapper/jfdctint-12.c
		libjpeg-turbo/src/wrapper/jfdctint-8.c
		libjpeg-turbo/src/wrapper/jidctflt-12.c
		libjpeg-turbo/src/wrapper/jidctflt-8.c
		libjpeg-turbo/src/wrapper/jidctfst-12.c
		libjpeg-turbo/src/wrapper/jidctfst-8.c
		libjpeg-turbo/src/wrapper/jidctint-12.c
		libjpeg-turbo/src/wrapper/jidctint-8.c
		libjpeg-turbo/src/wrapper/jidctred-12.c
		libjpeg-turbo/src/wrapper/jidctred-8.c
		libjpeg-turbo/src/wrapper/jquant1-12.c
		libjpeg-turbo/src/wrapper/jquant1-8.c
		libjpeg-turbo/src/wrapper/jquant2-12.c
		libjpeg-turbo/src/wrapper/jquant2-8.c
		libjpeg-turbo/src/wrapper/jutils-12.c
		libjpeg-turbo/src/wrapper/jutils-8.c
	)
	target_include_directories(libjpeg_turbo PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libjpeg_turbo/src>
		$<INSTALL_INTERFACE:include/libjpeg>
	)
	set_target_properties(libjpeg_turbo PROPERTIES POSITION_INDEPENDENT_CODE ON)
	install(TARGETS libjpeg_turbo EXPORT AllegroTargets)

	set(JPEG_FOUND TRUE PARENT_SCOPE)
	set(JPEG_LIBRARIES libjpeg_turbo PARENT_SCOPE)
	set(JPEG_LIBRARY libjpeg_turbo PARENT_SCOPE)
	set(JPEG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/src" PARENT_SCOPE)
	set(JPEG_INCLUDE_DIRS "${JPEG_INCLUDE_DIR}" PARENT_SCOPE)
	set(SUPPORT_JPEG 1 CACHE INTERNAL "Force jpeg support in Allegro")
endif()
