#CMake build, Pedro Vicente

#/////////////////////////////////////////////////////////////////////////////////////
#library dependencies: netCDF, HDF5, HDF5_HL, ZLIB (SZIP)
#/////////////////////////////////////////////////////////////////////////////////////

#-DNETCDF_INCLUDE:PATH=<some_path>
#-DNETCDF_LIBRARY:FILE=<some_file>
#-DHDF5_LIBRARY:FILE=<some_file>
#-DHDF5_HL_LIBRARY:FILE=<some_file>
#-DZLIB_LIBRARY:FILE=<some_file>
#-DSZIP_LIBRARY:FILE=<some_file>
#-DCURL_LIBRARY:FILE=<some_file>

cmake_minimum_required(VERSION 2.8)
project (star-json)
include(CheckCXXCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)

#/////////////////////////////////////////////////////////////////////////////////////
#general C/C++ build flags
#/////////////////////////////////////////////////////////////////////////////////////

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

#//////////////////////////
# colorized output
# red, fatal error, cannot build (missing netCDF, HDF5, CURL libraries)
# magenta, "negative" warning (missing optional libraries, GSL, ANTLR, UDUNITS)
# green, "positive" warning (opposite of magenta, optional libraries found)
# blue, general information to take notice (SZIP/ZLIB were detected as needed, special NCO functions are defined)
#//////////////////////////

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(color_reset   "${Esc}[m")
  set(color_magenta "${Esc}[35m")
  set(color_red     "${Esc}[31m")
  set(color_green   "${Esc}[32m")
  set(color_blue    "${Esc}[34m")
endif()

if (MSVC)
  add_definitions( "/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE" )
  message("-- Building with static runtime library")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT /O1 /Ob1 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /D NDEBUG")
endif(MSVC)

if (CMAKE_VERSION VERSION_LESS "3.1")
 if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
   CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
   if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
   endif()
 endif()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif()

#//////////////////////////
#detect dependencies using
#find_path()
#find_library()
#//////////////////////////

#/////////////////////////////////////////////////////////////////////////////////////
#HDF5
#/////////////////////////////////////////////////////////////////////////////////////

if(HDF5_INCLUDE)
  if(EXISTS "${HDF5_INCLUDE}/hdf5.h")
  else()
    message(FATAL_ERROR "${color_red}hdf5.h not found at ${HDF5_INCLUDE}${color_reset}")
  endif()
endif()

find_path(HDF5_INCLUDE hdf5.h)
if(HDF5_INCLUDE)
  message("-- Found hdf5.h header file at: " ${HDF5_INCLUDE})
  add_definitions(-DHAVE_HDF5)
else()
  message("${color_red}-- hdf5.h header file not found...disabling HDF5${color_reset}")
endif()

if(HDF5_INCLUDE)

  find_path(HDF5_HL_INCLUDE hdf5_hl.h)
  if(HDF5_HL_INCLUDE)
   message("-- Found hdf5_hl.h header file at: " ${HDF5_HL_INCLUDE})
  else()
   message(FATAL_ERROR "${color_red}-- hdf5_hl.h header file not found${color_reset}")
  endif()

  if (HDF5_LIBRARY)
    if(EXISTS "${HDF5_LIBRARY}")
    else()
      message(FATAL_ERROR "${color_red}Specified ${HDF5_LIBRARY} does not exist${color_reset}")
    endif()
  endif()

  find_library(HDF5_LIBRARY NAMES hdf5)
  if(NOT HDF5_LIBRARY)
    message(FATAL_ERROR "${color_red}hdf5 library not found${color_reset}")
  else()
    message("-- Found hdf5 library at: " ${HDF5_LIBRARY})
  endif()
  
  find_library(HDF5_HL_LIBRARY NAMES hdf5_hl ${find_opt})
  if(NOT HDF5_HL_LIBRARY)
    message(FATAL_ERROR "${color_red}hdf5 high level library not found${color_reset}")
  else()
    message("-- Found hdf5 high level library at: " ${HDF5_HL_LIBRARY})
  endif()

  #//////////////////////////
  #HDF5 can be optionally linked with the SZIP library (Science Data Lossless Compression Program) and ZLIB
  #Symbol to detect in ZLIB can be only H5Z_DEFLATE, a structure only defined in H5Zdeflate.c if the filter is enabled
  #For SZIP the structure can be only H5Z_SZIP, defined in H5Zszip.c if the filter is enabled
  #check_library_exists() tries to link a temporary program with these symbols
  #On MSVC for this detection to work, the HDF5 library MUST HAVE as additional dependencies the ZLIB and SZIP libraries,
  #which is not a requirement for the library to build successfully
  #//////////////////////////

  set(CMAKE_REQUIRED_LIBRARIES ${HDF5_LIBRARY})

  message("-- Detecting if HDF5 library ${HDF5_LIBRARY} needs the ZLIB library...")
  check_library_exists(${HDF5_LIBRARY} H5Z_DEFLATE "" NEED_ZLIB)
  if (NEED_ZLIB)
    message("${color_blue}-- ZLIB library is needed...${color_reset}")
  else()
    message("${color_blue}-- ZLIB library is not needed...${color_reset}")
  endif()

  message("-- Detecting if HDF5 library ${HDF5_LIBRARY} needs the SZIP library...")
  check_library_exists(${HDF5_LIBRARY} H5Z_SZIP "" NEED_SZIP)
  if (NEED_SZIP)
    message("${color_blue}-- SZIP library is needed...${color_reset}")
  else()
    message("${color_blue}-- SZIP library is not needed...${color_reset}")
  endif()

  if (NEED_SZIP)
    message("-- Trying to find the SZIP library...")
    if (SZIP_LIBRARY)
      if(EXISTS "${SZIP_LIBRARY}")
      else()
        message(FATAL_ERROR "${color_red}Specified ${SZIP_LIBRARY} does not exist${color_reset}")
      endif()
    endif()
    find_library(SZIP_LIBRARY NAMES sz ${find_opt})
    if(NOT SZIP_LIBRARY)
      message(FATAL_ERROR "${color_red}szip library not found${color_reset}")
    else()
      message("-- Found szip library at: " ${SZIP_LIBRARY})
    endif()
  endif()

  if (NEED_ZLIB)
    message("-- Trying to find the ZLIB library...")
    if (ZLIB_LIBRARY)
      if(EXISTS "${ZLIB_LIBRARY}")
      else()
        message(FATAL_ERROR "${color_red}Specified ${ZLIB_LIBRARY} does not exist${color_reset}")
      endif()
    endif()
    find_library(ZLIB_LIBRARY NAMES libz.a z ${find_opt})
    if(NOT ZLIB_LIBRARY)
      message(FATAL_ERROR "${color_red}zlib library not found${color_reset}")
    else()
      message("-- Found zlib library at: " ${ZLIB_LIBRARY})
    endif()
  endif()

  #//////////////////////////
  #link with libraries
  #LIB_DEP contains a cascade definition of all the libraries needed to link
  #//////////////////////////

  set(LIB_DEP ${LIB_DEP})
  set(LIB_DEP ${LIB_DEP} ${HDF5_LIBRARY})
  set(LIB_DEP ${LIB_DEP} ${HDF5_HL_LIBRARY})
  if (NEED_ZLIB)
    set(LIB_DEP ${LIB_DEP} ${ZLIB_LIBRARY})
  endif()
  if (NEED_SZIP)
    set(LIB_DEP ${LIB_DEP} ${SZIP_LIBRARY})
  endif()

  #//////////////////////////
  #include_directories
  #//////////////////////////

  include_directories(${HDF5_INCLUDE})
  include_directories(${HDF5_HL_INCLUDE})

endif(HDF5_INCLUDE)

include_directories(gason/src)
include_directories(gasonpp/src)

#//////////////////////////
# system specific libraries needed to link (assume existing)
#//////////////////////////

if(MSVC)
  set(LIB_DEP ${LIB_DEP} ws2_32.lib winmm.lib wldap32.lib)
endif()

set(sources ${sources})
set(sources ${sources} star_json.hh)
set(sources ${sources} star_json.cc)
set(sources ${sources} star_dataset.hh)
set(sources ${sources} star_dataset.cc)
set(sources ${sources} gason/src/gason.h)
set(sources ${sources} gason/src/gason.cpp)
set(sources ${sources} gasonpp/src/jsonbuilder.hpp)
add_library(star_json STATIC ${sources})
set(star_json_LIB star_json)
add_executable(parser parser.cc)
target_link_libraries (parser ${star_json_LIB} ${LIB_DEP})
if(HDF5_INCLUDE)
add_executable(reader_atms reader_atms.cc ${sources} read_atms.hh read_atms.cc)
target_link_libraries (reader_atms ${star_json_LIB} ${LIB_DEP})
endif(HDF5_INCLUDE)



