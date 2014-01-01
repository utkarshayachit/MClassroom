cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(QtClassroom)
# enable @rpath in the install name for any shared library being built
# note: it is planned that a future version of CMake will enable this by default
set(CMAKE_MACOSX_RPATH 1)


set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

find_package(Qt4 4.8 REQUIRED QtCore QtGui QtNetwork)
include(${QT_USE_FILE})


set(Boost_USE_STATIC_LIBS  ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED thread system
                            timer chrono)

#------------------------------------------------------------------------------

# RTAudio is used for Audio runtime.
find_library(RTAUDIO_LIBRARY NAMES rtaudio)
find_path(RTAUDIO_INCLUDE_DIR NAMES RtAudio.h)

if (NOT RTAUDIO_INCLUDE_DIR OR NOT RTAUDIO_LIBRARY)
  message(FATAL "Missing RtAudio.")
endif()


# Opus is used for compressing audio data from transmission.
find_library(OPUS_LIBRARY NAMES opus)
find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h)

if (NOT OPUS_INCLUDE_DIR OR NOT OPUS_LIBRARY)
  message(FATAL "Missing Opus")
endif()


# libnice is used for communication between machines behind firewalls/NATS.
find_library(NICE_LIBRARY NAMES nice)
find_path(NICE_INCLUDE_DIR NAMES nice/nice.h)

if (NOT NICE_INCLUDE_DIR OR NOT NICE_LIBRARY)
  message(FATAL "Missing libnice")
endif()

# nice needs glib2
#find_package(GLIB2 REQUIRED)
#if (NOT GLIB2_FOUND)
#  message(FATAL "GLIB2 is required by libnice")
#endif()

qt4_wrap_cpp(moc_srs
  qcMainWindow.h
  qcQUdpNetworkProtocol.h
  qcNiceProtocol.h
)

qt4_wrap_ui(ui_srcs
  qcMainWindow.ui
  qcNiceConnectionDialog.ui
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${RTAUDIO_INCLUDE_DIR}
                    ${OPUS_INCLUDE_DIR}
                    ${NICE_INCLUDE_DIR}
        #            ${GLIB2_INCLUDE_DIRS}
                    /usr/local/Cellar/glib/2.38.2/include/glib-2.0
                    /usr/local/Cellar/glib/2.38.2/lib/glib-2.0/include
                    /usr/local/opt/gettext/include
                    )
add_executable(QtClassroom MACOSX_BUNDLE
  qcApp.cxx
  qcApp.h
  qcMainWindow.cxx
  qcMainWindow.h
  qcQUdpNetworkProtocol.h
  qcReceiver.h
  qcNiceProtocol.h
  qcNiceProtocol.cxx
  ${moc_srs}
  ${ui_srcs})
target_link_libraries(QtClassroom ${QT_LIBRARIES}
  ${RTAUDIO_LIBRARY}
  ${OPUS_LIBRARY}
  ${Boost_LIBRARIES}
  ${NICE_LIBRARY}
#  ${GLIB2_LIBRARIES}
  /usr/local/Cellar/glib/2.38.2/lib/libglib-2.0.dylib
  /usr/local/Cellar/glib/2.38.2/lib/libgobject-2.0.dylib
  /usr/local/Cellar/glib/2.38.2/lib/libgio-2.0.dylib
  )
