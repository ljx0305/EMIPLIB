cmake_minimum_required(VERSION 2.8)
if (POLICY CMP0020) # automatically link to qtmain on windows
   	cmake_policy(SET CMP0020 NEW)
endif()
if (POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif()

project(emiplib)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_DEBUG 1)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_DEBUG}")

include(CheckCXXCompilerFlag)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
	# Test if the compiler already supports C++11
	check_cxx_source_compiles("#include <unordered_map>\nint main(void)\n{\nstd::unordered_map<int, int> m { { 1, 111}, {2, 222} };\n return 0; \n }" C11AVAIL)
	if (NOT C11AVAIL)
		message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
	endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
if (CMAKE_INSTALL_PREFIX AND NOT CMAKE_PREFIX_PATH)
    #message("Setting CMAKE_PREFIX_PATH to ${CMAKE_INSTALL_PREFIX}")
    file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" CMAKE_PREFIX_PATH)
endif()

set (_DEFAULT_LIBRARY_INSTALL_DIR lib)
if (EXISTS "${CMAKE_INSTALL_PREFIX}/lib32/" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
	set (_DEFAULT_LIBRARY_INSTALL_DIR lib32)
elseif (EXISTS "${CMAKE_INSTALL_PREFIX}/lib64/" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	set (_DEFAULT_LIBRARY_INSTALL_DIR lib64)
endif ()

set(LIBRARY_INSTALL_DIR "${_DEFAULT_LIBRARY_INSTALL_DIR}" CACHE PATH "Library installation directory")
if(NOT IS_ABSOLUTE "${LIBRARY_INSTALL_DIR}")
	set(LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIBRARY_INSTALL_DIR}")
endif()

include(TestBigEndian)
include("${PROJECT_SOURCE_DIR}/cmake/Macros.cmake")

if (MSVC)
	set(EMIPLIB_COMPILE_STATIC ON CACHE BOOL "Flag indicating if a static library should be built, or a dynamic one")
endif ()

if (NOT MSVC)
	if (APPLE)
		set(TMP ON)
	else (APPLE)
		set(TMP OFF)
	endif (APPLE)
	option(EMIPLIB_LINK "Flag indicating if the shared library should be automatically linked against the other libraries used" ${TMP})
else ()
	if (EMIPLIB_COMPILE_STATIC)
		set(EMIPLIB_LINK FALSE)
	else (EMIPLIB_COMPILE_STATIC)
		set(EMIPLIB_LINK TRUE)
	endif (EMIPLIB_COMPILE_STATIC)
endif ()

set(EMIPLIB_REQUIRED_MODULES "")
set(EMIPLIB_INTERNAL_INCLUDES "")
set(EMIPLIB_EXTERNAL_INCLUDES "")
set(EMIPLIB_LINK_LIBS "")

add_additional_stuff(EMIPLIB_EXTERNAL_INCLUDES EMIPLIB_LINK_LIBS)

find_package(JThread REQUIRED)
find_package(JRTPLIB REQUIRED)

list(APPEND EMIPLIB_EXTERNAL_INCLUDES "${JTHREAD_INCLUDE_DIRS}")
list(APPEND EMIPLIB_EXTERNAL_INCLUDES "${JRTPLIB_INCLUDE_DIRS}")
list(APPEND EMIPLIB_LINK_LIBS "${JTHREAD_LIBRARIES}")
list(APPEND EMIPLIB_LINK_LIBS "${JRTPLIB_LIBRARIES}")

# Make sure jrtplib is recent enough
#try_compile(JRTP_RESULT "${PROJECT_BINARY_DIR}/jrtplibversiontest" "${PROJECT_SOURCE_DIR}/tools/jrtplibversion" 
#	jrtplibversiontest CMAKE_FLAGS 
#	"-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DJThread_DIR:PATH=${JThread_DIR} -DJRTPLIB_DIR:PATH=${JRTPLIB_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}" OUTPUT_VARIABLE OUTVAR)
#if (NOT JRTP_RESULT)
#	message(FATAL_ERROR "JRTPLIB was detected, but the version does not seem to be recent enough.\nCompilation log:\n${OUTVAR}\n")
#endif()

option(EMIPLIB_COMPILE_TESTS "Compile various tests in the 'tests' subdirectory" NO)
emiplib_support_option("Support for GPL components. This changes the license to GPL instead of LGPL."
	               EMIPLIB_GPL MIPCONFIG_GPL OFF "// No GPL components will be compiled")
emiplib_support_option("Support for the internal GSM codec" EMIPLIB_SUPPORT_GSM MIPCONFIG_SUPPORT_GSM ON "// No GSM support")
emiplib_support_option("Support for the internal LPC codec" EMIPLIB_SUPPORT_LPC MIPCONFIG_SUPPORT_LPC ON "// No LPC support")
emiplib_support_option("Support for Intel IPP library" EMIPLIB_SUPPORT_INTELIPP MIPCONFIG_SUPPORT_INTELIPP OFF "// No support for Intel IPP library")
emiplib_support_option("Support for SILK codec" EMIPLIB_SUPPORT_SILK MIPCONFIG_SUPPORT_SILK OFF "// No support for SILK codec")
emiplib_support_option("Support for Android AudioTrack output component" EMIPLIB_SUPPORT_AUDIOTRACK MIPCONFIG_SUPPORT_AUDIOTRACK OFF "// No support for AudioTrack output component")
emiplib_support_option("Support for Android AudioRecorder input component" EMIPLIB_SUPPORT_AUDIORECORDER MIPCONFIG_SUPPORT_AUDIORECORDER OFF "// No support for AudioRecorder input component")

emiplib_test_feature_option(osstest MIPCONFIG_SUPPORT_OSS "// No support for OSS" EMIPLIB_SUPPORT_OSS "Support for Open Sound System input/output")
emiplib_test_feature_option(v4ltest MIPCONFIG_SUPPORT_VIDEO4LINUX "// No support for Video4Linux" EMIPLIB_SUPPORT_VIDEO4LINUX "Support for Video4Linux input")
emiplib_test_feature_option(openslesandroid MIPCONFIG_SUPPORT_OPENSLESANDROID "// No support for OpenSL ES on Android" EMIPLIB_SUPPORT_OPENSLESANDROID "Support for OpenSL ES on Android")
if (EMIPLIB_SUPPORT_OPENSLESANDROID)
	list(APPEND EMIPLIB_LINK_LIBS "-lOpenSLES")
endif (EMIPLIB_SUPPORT_OPENSLESANDROID)

emiplib_test_feature_option(winmmtest MIPCONFIG_SUPPORT_WINMM "// No support for Win32 multimedia input/output" EMIPLIB_SUPPORT_WINMM "Support for Win32 multimedia input/output")
if (EMIPLIB_SUPPORT_WINMM)
	list(APPEND EMIPLIB_LINK_LIBS "winmm")
endif (EMIPLIB_SUPPORT_WINMM)

emiplib_support_option("Support for DirectShow" EMIPLIB_SUPPORT_DIRECTSHOW MIPCONFIG_SUPPORT_DIRECTSHOW OFF "// No support for DirectShow video input codec")

find_package(Speex)
if (SPEEX_FOUND)
	set(TMP ON)
else (SPEEX_FOUND)
	set(TMP OFF)
endif (SPEEX_FOUND)
emiplib_support_option("Support for Speex audio codec" EMIPLIB_SUPPORT_SPEEX MIPCONFIG_SUPPORT_SPEEX ${TMP} "// No Speex support")
if (EMIPLIB_SUPPORT_SPEEX AND SPEEX_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${SPEEX_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${SPEEX_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_SPEEX AND SPEEX_FOUND)

find_package(Opus)
if (OPUS_FOUND)
	set(TMP ON)
else (OPUS_FOUND)
	set(TMP OFF)
endif (OPUS_FOUND)
emiplib_support_option("Support for Opus audio codec" EMIPLIB_SUPPORT_OPUS MIPCONFIG_SUPPORT_OPUS ${TMP} "// No Opus support")
if (EMIPLIB_SUPPORT_OPUS AND OPUS_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${OPUS_LIBRARIES})
	list(APPEND EMIPLIB_INTERNAL_INCLUDES ${OPUS_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_OPUS AND OPUS_FOUND)

find_package(ALSA)
if (ALSA_FOUND)
	set(TMP ON)
else (ALSA_FOUND)
	set(TMP OFF)
endif (ALSA_FOUND)
emiplib_support_option("Support for ALSA" EMIPLIB_SUPPORT_ALSA MIPCONFIG_SUPPORT_ALSA ${TMP} "// No ALSA support")
if (EMIPLIB_SUPPORT_ALSA AND ALSA_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${ALSA_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${ALSA_INCLUDE_DIRS})
endif(EMIPLIB_SUPPORT_ALSA AND ALSA_FOUND)

find_package(PortAudio)
if (PORTAUDIO_FOUND)
	set(TMP ON)
else (PORTAUDIO_FOUND)
	set(TMP OFF)
endif (PORTAUDIO_FOUND)
emiplib_support_option("Support for PortAudio input/output" EMIPLIB_SUPPORT_PORTAUDIO MIPCONFIG_SUPPORT_PORTAUDIO ${TMP} "// No PortAudio support")
if (EMIPLIB_SUPPORT_PORTAUDIO AND PORTAUDIO_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${PORTAUDIO_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${PORTAUDIO_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_PORTAUDIO AND PORTAUDIO_FOUND)

find_package(OpenAL)
if (OPENAL_FOUND)
	set(TMP ON)
else (OPENAL_FOUND)
	set(TMP OFF)
endif (OPENAL_FOUND)
emiplib_support_option("Support for OpenAL output" EMIPLIB_SUPPORT_OPENAL MIPCONFIG_SUPPORT_OPENAL ${TMP} "// No OpenAL output support")
if (EMIPLIB_SUPPORT_OPENAL AND OPENAL_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${OPENAL_LIBRARY})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${OPENAL_INCLUDE_DIR})
endif (EMIPLIB_SUPPORT_OPENAL AND OPENAL_FOUND)

find_package(LibSndFile)
if (LIBSNDFILE_FOUND)
	set(TMP ON)
else (LIBSNDFILE_FOUND)
	set(TMP OFF)
endif (LIBSNDFILE_FOUND)
emiplib_support_option("Support for libsndfile audio file input/output" EMIPLIB_SUPPORT_SNDFILE MIPCONFIG_SUPPORT_SNDFILE ${TMP} "// No libsndfile support")
if (EMIPLIB_SUPPORT_SNDFILE AND LIBSNDFILE_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${LIBSNDFILE_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${LIBSNDFILE_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_SNDFILE AND LIBSNDFILE_FOUND)

find_package(AudioFile)
if (AUDIOFILE_FOUND)
	set(TMP ON)
else (AUDIOFILE_FOUND)
	set(TMP OFF)
endif (AUDIOFILE_FOUND)
emiplib_support_option("Support for libaudiofile audio file input" EMIPLIB_SUPPORT_AUDIOFILE MIPCONFIG_SUPPORT_AUDIOFILE ${TMP} "// No libaudiofile support")
if (EMIPLIB_SUPPORT_AUDIOFILE AND AUDIOFILE_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${AUDIOFILE_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${AUDIOFILE_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_AUDIOFILE AND AUDIOFILE_FOUND)

find_package(SDL)
if (SDL_FOUND)
	set(TMP ON)
else (SDL_FOUND)
	set(TMP OFF)
endif (SDL_FOUND)
emiplib_support_option("Support for SDL audio output" EMIPLIB_SUPPORT_SDLAUDIO MIPCONFIG_SUPPORT_SDLAUDIO ${TMP} "// No support for SDL audio output")
if (EMIPLIB_SUPPORT_SDLAUDIO AND SDL_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${SDL_LIBRARY})
	list(APPEND EMIPLIB_INTERNAL_INCLUDES ${SDL_INCLUDE_DIR}) # Only needed in a cpp file
endif (EMIPLIB_SUPPORT_SDLAUDIO AND SDL_FOUND)

find_package(JACK)
if (JACK_FOUND)
	set(TMP ON)
else (JACK_FOUND)
	set(TMP OFF)
endif (JACK_FOUND)
emiplib_support_option("Support for JACK audio input/output" EMIPLIB_SUPPORT_JACK MIPCONFIG_SUPPORT_JACK ${TMP} "// No support for JACK audio input/output")
if (EMIPLIB_SUPPORT_JACK AND JACK_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${JACK_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${JACK_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_JACK AND JACK_FOUND)

find_package(ESound)
if (ESOUND_FOUND)
	set(TMP ON)
else (ESOUND_FOUND)
	set(TMP OFF)
endif (ESOUND_FOUND)
emiplib_support_option("Support for the Enlightened Sound Daemon based output" EMIPLIB_SUPPORT_ESD MIPCONFIG_SUPPORT_ESD ${TMP} "// No support for ESD based output")
if (EMIPLIB_SUPPORT_ESD AND ESOUND_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${ESOUND_LIBRARIES})
	list(APPEND EMIPLIB_INTERNAL_INCLUDES ${ESOUND_INCLUDE_DIRS}) # Only needed in a cpp file
endif (EMIPLIB_SUPPORT_ESD AND ESOUND_FOUND)

find_package(LibAVCodec)
if (LIBAVCODEC_FOUND)
	set(TMP ON)
else (LIBAVCODEC_FOUND)
	set(TMP OFF)
endif (LIBAVCODEC_FOUND)
emiplib_support_option("Support for libavcodec for video compression/decompression" EMIPLIB_SUPPORT_AVCODEC MIPCONFIG_SUPPORT_AVCODEC ${TMP} "// No libavcodec support")
if (EMIPLIB_SUPPORT_AVCODEC AND LIBAVCODEC_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${LIBAVCODEC_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${LIBAVCODEC_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_AVCODEC AND LIBAVCODEC_FOUND)

find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(Qt5Multimedia)
if (Qt5Gui_FOUND AND Qt5Widgets_FOUND AND Qt5OpenGL_FOUND AND Qt5Multimedia_FOUND)
	set(TMP ON)
else ()
	set(TMP OFF)
endif ()
emiplib_support_option("Support for Qt5 output components" EMIPLIB_SUPPORT_QT5 MIPCONFIG_SUPPORT_QT5 ${TMP} "// No support for Qt5 output components")
if (EMIPLIB_SUPPORT_QT5 AND Qt5Gui_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS "${Qt5Gui_LIBRARIES}")
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES "${Qt5Gui_INCLUDE_DIRS}")
	set(EMIPLIB_REQUIRED_MODULES "${EMIPLIB_REQUIRED_MODULES}\nfind_package(Qt5Gui REQUIRED)")

	set(CMAKE_AUTOMOC ON)
endif (EMIPLIB_SUPPORT_QT5 AND Qt5Gui_FOUND)

if (EMIPLIB_SUPPORT_QT5 AND Qt5Widgets_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS "${Qt5Widgets_LIBRARIES}")
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES "${Qt5Widgets_INCLUDE_DIRS}")
	set(EMIPLIB_REQUIRED_MODULES "${EMIPLIB_REQUIRED_MODULES}\nfind_package(Qt5Widgets REQUIRED)")
endif (EMIPLIB_SUPPORT_QT5 AND Qt5Widgets_FOUND)

if (EMIPLIB_SUPPORT_QT5 AND Qt5OpenGL_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS "${Qt5OpenGL_LIBRARIES}")
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES "${Qt5OpenGL_INCLUDE_DIRS}")
	set(EMIPLIB_REQUIRED_MODULES "${EMIPLIB_REQUIRED_MODULES}\nfind_package(Qt5OpenGL REQUIRED)")
endif (EMIPLIB_SUPPORT_QT5 AND Qt5OpenGL_FOUND)

if (EMIPLIB_SUPPORT_QT5 AND Qt5Multimedia_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS "${Qt5Multimedia_LIBRARIES}")
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES "${Qt5Multimedia_INCLUDE_DIRS}")
	set(EMIPLIB_REQUIRED_MODULES "${EMIPLIB_REQUIRED_MODULES}\nfind_package(Qt5Multimedia REQUIRED)")
endif (EMIPLIB_SUPPORT_QT5 AND Qt5Multimedia_FOUND)

find_package(PulseAudio)
if (PULSEAUDIO_FOUND)
	set(TMP ON)
else (PULSEAUDIO_FOUND)
	set(TMP OFF)
endif (PULSEAUDIO_FOUND)
emiplib_support_option("Support for PulseAudio input/output" EMIPLIB_SUPPORT_PULSEAUDIO MIPCONFIG_SUPPORT_PULSEAUDIO ${TMP} "// No support for PulseAudio input/output")
if (EMIPLIB_SUPPORT_PULSEAUDIO AND PULSEAUDIO_FOUND)
	list(APPEND EMIPLIB_LINK_LIBS ${PULSEAUDIO_LIBRARIES})
	list(APPEND EMIPLIB_EXTERNAL_INCLUDES ${PULSEAUDIO_INCLUDE_DIRS})
endif (EMIPLIB_SUPPORT_PULSEAUDIO AND PULSEAUDIO_FOUND)

if (CMAKE_CROSSCOMPILING)
	option (EMIPLIB_USE_BIGENDIAN "Target platform is big endian" ON)
	if (EMIPLIB_USE_BIGENDIAN)
		set(MIPCONFIG_BIGENDIAN "#define MIPCONFIG_BIGENDIAN")
	else (EMIPLIB_USE_BIGENDIAN)
		set(MIPCONFIG_BIGENDIAN "// Little endian system")
	endif (EMIPLIB_USE_BIGENDIAN)
else (CMAKE_CROSSCOMPILING)
	test_big_endian(EMIPLIB_BIGENDIAN)
	if (EMIPLIB_BIGENDIAN)
		set(MIPCONFIG_BIGENDIAN "#define MIPCONFIG_BIGENDIAN")
	else (EMIPLIB_BIGENDIAN)
		set(MIPCONFIG_BIGENDIAN "// Little endian system")
	endif (EMIPLIB_BIGENDIAN)
endif (CMAKE_CROSSCOMPILING)

set(EMIPLIB_INTERNAL_INCLUDES ${EMIPLIB_INTERNAL_INCLUDES}
	"${PROJECT_SOURCE_DIR}/src/core"
	"${PROJECT_BINARY_DIR}/"
	"${PROJECT_BINARY_DIR}/src/core"
	"${PROJECT_SOURCE_DIR}/src/components/input"
	"${PROJECT_SOURCE_DIR}/src/components/output"
	"${PROJECT_SOURCE_DIR}/src/components/output"
	"${PROJECT_SOURCE_DIR}/src/components/timer"
	"${PROJECT_SOURCE_DIR}/src/components/mixer"
	"${PROJECT_SOURCE_DIR}/src/components/io"
	"${PROJECT_SOURCE_DIR}/src/components/transmission"
	"${PROJECT_SOURCE_DIR}/src/components/transform"
	"${PROJECT_SOURCE_DIR}/src/components/codec"
	"${PROJECT_SOURCE_DIR}/src/components/util"
	"${PROJECT_SOURCE_DIR}/src/sessions"
	"${PROJECT_SOURCE_DIR}/src/util"
	"${PROJECT_SOURCE_DIR}/src/thirdparty/gsm/inc"
	"${PROJECT_SOURCE_DIR}/src/thirdparty/lpc"
	"${PROJECT_SOURCE_DIR}/src/thirdparty/tinyjpeg" )

check_cxx_source_compiles("#include <stdint.h>\nint main(void) { uint32_t x = 0; uint32_t y = x; return 0; }" EMIPLIB_STDINT)
if (EMIPLIB_STDINT)
	set(EMIPLIB_INTTYPE_HEADERS "#include <stdint.h>\n")
else (EMIPLIB_STDINT)
	check_cxx_source_compiles("#include <inttypes.h>\nint main(void) { uint32_t x = 0; uint32_t y = x; return 0; }" EMIPLIB_INTTYPES)
	if (EMIPLIB_INTTYPES)
		set(EMIPLIB_INTTYPE_HEADERS "#include <inttypes.h>\n")
	else (EMIPLIB_INTTYPES)
		if (MSVC)
			set(EMIPLIB_INTTYPE_HEADERS "#include \"miptypes_win.h\"")
		else ()
			set(EMIPLIB_INTTYPE_HEADERS "#error Could not find header files that define types like 'uint32_t'. Please edit the file ${PROJECT_BINARY_DIR}/src/core/miptypes.h and make sure that the following types are defined: int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t")
			message("\n\nError: Could not find header files that define types like 'uint32_t'.\nPlease edit the file ${PROJECT_BINARY_DIR}/src/core/miptypes.h\nand make sure that the following types are defined: \nint8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t\n\n")
		endif ()
	endif (EMIPLIB_INTTYPES)
endif (EMIPLIB_STDINT)

if (NOT MSVC OR EMIPLIB_COMPILE_STATIC)
	set(EMIPLIB_IMPORT "")
	set(EMIPLIB_EXPORT "")
else ()
	set(EMIPLIB_IMPORT "__declspec(dllimport)")
	set(EMIPLIB_EXPORT "__declspec(dllexport)")
endif ()

configure_file("${PROJECT_SOURCE_DIR}/src/core/mipconfig.h.in" "${PROJECT_BINARY_DIR}/src/core/mipconfig.h")
configure_file("${PROJECT_SOURCE_DIR}/src/core/miptypes.h.in" "${PROJECT_BINARY_DIR}/src/core/miptypes.h")
configure_file("${PROJECT_SOURCE_DIR}/src/core/mipversioninternal.h.in" "${PROJECT_BINARY_DIR}/src/core/mipversioninternal.h")

add_subdirectory(src)
add_subdirectory(examples)

if (EMIPLIB_COMPILE_TESTS)
	add_subdirectory(tests)
endif()

if (NOT MSVC)
	set(EMIPLIB_LIBS "-L${LIBRARY_INSTALL_DIR}" "-lemip")
else ()
	set(EMIPLIB_LIBS optimized "${LIBRARY_INSTALL_DIR}/emiplib.lib" 
	                 debug "${LIBRARY_INSTALL_DIR}/emiplib_d.lib")
endif ()

set(EMIPLIB_INCDIRS ${EMIPLIB_EXTERNAL_INCLUDES} ${CMAKE_INSTALL_PREFIX}/include)
set(EMIPLIB_LIBS ${EMIPLIB_LIBS} ${EMIPLIB_LINK_LIBS})
remove_empty(EMIPLIB_INCDIRS)
list(REMOVE_DUPLICATES EMIPLIB_INCDIRS)
remove_empty(EMIPLIB_LIBS) # Note that we can't remove duplicates because of multiple 'optimized' or 'debug' options in Win32 version

foreach(ARG ${EMIPLIB_LIBS})
	set(EMIPLIB_LIBS_CMAKECONFIG "${EMIPLIB_LIBS_CMAKECONFIG} \"${ARG}\"")
endforeach()
foreach(ARG ${EMIPLIB_INCDIRS})
	set(EMIPLIB_INCDIRS_CMAKECONFIG "${EMIPLIB_INCDIRS_CMAKECONFIG} \"${ARG}\"")
endforeach()

configure_file("${PROJECT_SOURCE_DIR}/cmake/EMIPLIBConfig.cmake.in" "${PROJECT_BINARY_DIR}/cmake/EMIPLIBConfig.cmake")
install(FILES "${PROJECT_BINARY_DIR}/cmake/EMIPLIBConfig.cmake" DESTINATION ${LIBRARY_INSTALL_DIR}/cmake/EMIPLIB)

if (NOT MSVC)
	foreach(ARG ${EMIPLIB_LIBS})
		set(EMIPLIB_LIBS_PKGCONFIG "${EMIPLIB_LIBS_PKGCONFIG} ${ARG}")
	endforeach()
	foreach(ARG ${EMIPLIB_INCDIRS})
		set(EMIPLIB_INCDIRS_PKGCONFIG "${EMIPLIB_INCDIRS_PKGCONFIG} -I${ARG}")
	endforeach()

	configure_file(${PROJECT_SOURCE_DIR}/pkgconfig/emiplib.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/emiplib.pc)
	install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/emiplib.pc DESTINATION ${LIBRARY_INSTALL_DIR}/pkgconfig)
endif ()

if (EMIPLIB_GPL)
	message("License: GPL\n\n"
	"You have specified that you want to use components licensed under the terms\n"
	"of the GPL. By specifying this option, the GPL license also applies to this\n"
	"library. The complete text of the GPL license can be found in the file\n"
	"LICENSE.GPL included in the source code archive.\n")
else (EMIPLIB_GPL)
	message("License: LGPL\n\n"
	"The library is licensed under the terms of the LGPL, the full text can be\n"
	"found in the file LICENSE.LGPL. However, depending on the libraries your\n"
	"application links with, it may be possible that another license has to be\n"
	"taken into account as well. For example, if the libavcodec library you link\n"
	"with was compiled as a GPL library, then your application has to respect the\n"
	"terms of the GPL, even though it may only make use of libavcodec indirectly\n"
	"(by using the EMIPLIB libavcodec component). Similarly, if you use the Qt\n"
	"library with a GPL license, linking with the Qt library will cause your\n"
	"application to be GPL as well.\n")
endif (EMIPLIB_GPL)
	
