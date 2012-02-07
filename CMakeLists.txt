
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)
FIND_PACKAGE(Torch REQUIRED)

# make sure we find FindFFmpeg.cmake
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
FIND_PACKAGE(FFmpeg QUIET)

IF(FFMPEG_FOUND)
  include_directories (${FFMPEG_INCLUDE_DIR})
  set(src ffmpeg.c)
  set(luasrc init.lua)
  ADD_TORCH_PACKAGE(ffmpeglib "${src}" "${luasrc}" "FFMPEG lib wrapper")
  TARGET_LINK_LIBRARIES(ffmpeglib luaT TH ${FFMPEG_LIBRARIES})
ELSE(FFMPEG_FOUND)
  MESSAGE("WARNING: Could not find ffmpeg libraries.  ffmpeg lib wrapper will not be installed")
ENDIF(FFMPEG_FOUND)