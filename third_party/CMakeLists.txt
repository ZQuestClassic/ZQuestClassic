if(WANT_VENDORED_ZLIB)
	add_library(zlib STATIC
		zlib/src/adler32.c
		zlib/src/compress.c
		zlib/src/crc32.c
		zlib/src/deflate.c
		zlib/src/gzclose.c
		zlib/src/gzlib.c
		zlib/src/gzread.c
		zlib/src/gzwrite.c
		zlib/src/inffast.c
		zlib/src/inflate.c
		zlib/src/inftrees.c
		zlib/src/trees.c
		zlib/src/uncompr.c
		zlib/src/zutil.c
	)
	target_include_directories(zlib PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zlib/include>
		$<INSTALL_INTERFACE:include/zlib>
	)
	target_compile_definitions(zlib PRIVATE
		$<$<OR:$<CONFIG:Debug>,$<CONFIG:Asan>>:ZLIB_DEBUG>
	)
	if(UNIX)
		target_compile_definitions(zlib PRIVATE HAVE_UNISTD_H)
	endif()

	install(TARGETS zlib EXPORT AllegroTargets COMPONENT BuildOnly EXCLUDE_FROM_ALL)
	set_target_properties(zlib PROPERTIES POSITION_INDEPENDENT_CODE ON)

	set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
	set(ZLIB_LIBRARIES zlib CACHE STRING "" FORCE)
	set(ZLIB_LIBRARY zlib CACHE STRING "" FORCE)
	set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib/include" CACHE STRING "" FORCE)
	set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}" CACHE STRING "" FORCE)
	add_library(ZLIB::ZLIB ALIAS zlib)
else()
	find_package(ZLIB REQUIRED)
endif()

if(WANT_VENDORED_LIBPNG)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt"
		"${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h"
		COPYONLY
	)

	add_library(png STATIC
		libpng/src/png.c
		libpng/src/pngerror.c
		libpng/src/pngget.c
		libpng/src/pngmem.c
		libpng/src/pngpread.c
		libpng/src/pngread.c
		libpng/src/pngrio.c
		libpng/src/pngrtran.c
		libpng/src/pngrutil.c
		libpng/src/pngset.c
		libpng/src/pngtrans.c
		libpng/src/pngwio.c
		libpng/src/pngwrite.c
		libpng/src/pngwtran.c
		libpng/src/pngwutil.c
	)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
		target_sources(png PRIVATE
			libpng/src/intel/intel_init.c
			libpng/src/intel/filter_sse2_intrinsics.c
		)
		target_compile_definitions(png PUBLIC PNG_INTEL_SSE)
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|ARM|aarch64|AARCH64)")
		target_sources(png PRIVATE
			libpng/src/arm/arm_init.c
			libpng/src/arm/filter_neon_intrinsics.c
			libpng/src/arm/palette_neon_intrinsics.c
		)
		target_compile_definitions(png PRIVATE PNG_ARM_NEON)
	else()
		message(FATAL_ERROR "Unsupported processor: ${CMAKE_SYSTEM_PROCESSOR}")
	endif()
	target_include_directories(png PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libpng/src>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
		$<INSTALL_INTERFACE:include/png>
	)

	target_link_libraries(png PUBLIC ZLIB::ZLIB)

	install(TARGETS png EXPORT AllegroTargets COMPONENT BuildOnly EXCLUDE_FROM_ALL)
	set_target_properties(png PROPERTIES POSITION_INDEPENDENT_CODE ON)

	add_library(PNG::PNG ALIAS png)

	set(PNG_FOUND TRUE CACHE BOOL "" FORCE)
	set(PNG_LIBRARIES PNG::PNG CACHE STRING "" FORCE)
	set(PNG_LIBRARY PNG::PNG CACHE STRING "" FORCE)
	set(PNG_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/libpng/src;${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "" FORCE)
	set(PNG_PNG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libpng/src" CACHE STRING "" FORCE)
else()
	find_package(PNG REQUIRED)
endif()

if(WANT_VENDORED_DUMB)
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
	install(TARGETS dumb EXPORT AllegroTargets COMPONENT BuildOnly EXCLUDE_FROM_ALL)
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
	install(TARGETS libjpeg_turbo EXPORT AllegroTargets COMPONENT BuildOnly EXCLUDE_FROM_ALL)

	set(JPEG_FOUND TRUE PARENT_SCOPE)
	set(JPEG_LIBRARIES libjpeg_turbo PARENT_SCOPE)
	set(JPEG_LIBRARY libjpeg_turbo PARENT_SCOPE)
	set(JPEG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/src" PARENT_SCOPE)
	set(JPEG_INCLUDE_DIRS "${JPEG_INCLUDE_DIR}" PARENT_SCOPE)
	set(SUPPORT_JPEG 1 CACHE INTERNAL "Force jpeg support in Allegro")
endif()

# Remove once generally available in all compilers for c++ 20.
add_library(fmt STATIC
	fmt/src/format.cc
)
target_include_directories(fmt PUBLIC SYSTEM fmt/include)
if(WIN32)
	target_compile_options(fmt PUBLIC /utf-8)
endif()
