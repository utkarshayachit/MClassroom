cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(QtClassroom)
# enable @rpath in the install name for any shared library being built
# note: it is planned that a future version of CMake will enable this by default
set(CMAKE_MACOSX_RPATH 1)

find_package(Qt4 4.8 REQUIRED QtCore QtGui QtNetwork)
include(${QT_USE_FILE})


set(Boost_USE_STATIC_LIBS  ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED thread system
                            timer chrono)

#---------------------------------------------------------------------------------------------------------------------------------
find_library(RTAUDIO_LIBRARY NAMES rtaudio)
find_path(RTAUDIO_INCLUDE_DIR NAMES RtAudio.h)

find_library(OPUS_LIBRARY NAMES opus)
find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h)

if (NOT RTAUDIO_INCLUDE_DIR OR NOT RTAUDIO_LIBRARY)
  message(FATAL "Missing RtAudio.")
endif()

if (NOT OPUS_INCLUDE_DIR OR NOT OPUS_LIBRARY)
  message(FATAL "Missing Opus")
endif()

qt4_wrap_cpp(moc_srs
  qcMainWindow.h
  qcReceiver.h
)

qt4_wrap_ui(ui_srcs
  qcMainWindow.ui
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${RTAUDIO_INCLUDE_DIR}
                    ${OPUS_INCLUDE_DIR})
add_executable(QtClassroom MACOSX_BUNDLE
  qcApp.cxx
  qcApp.h
  qcMainWindow.cxx
  qcMainWindow.h
  qcReceiver.h
  ${moc_srs}
  ${ui_srcs})
target_link_libraries(QtClassroom ${QT_LIBRARIES}
  ${RTAUDIO_LIBRARY}
  ${OPUS_LIBRARY}
  ${Boost_LIBRARIES})
