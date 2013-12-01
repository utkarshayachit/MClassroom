cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
project(QtClassroom)

find_package(Qt4 4.8 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})


set(Boost_USE_STATIC_LIBS  ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED thread system)


#---------------------------------------------------------------------------------------------------------------------------------
find_library(RTAUDIO_LIBRARY NAMES librtaudio)
find_path(RTAUDIO_INCLUDE_DIR NAMES RtAudio.h)

if (NOT RTAUDIO_INCLUDE_DIR OR NOT RTAUDIO_LIBRARY)
  message(FATAL "Missing RtAudio.")
endif()

qt4_wrap_cpp(moc_srs
  qcMainWindow.h
)

qt4_wrap_ui(ui_srcs
  qcMainWindow.ui
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${RTAUDIO_INCLUDE_DIR})
add_executable(QtClassroom MACOSX_BUNDLE
  qcMainWindow.h
  qcMainWindow.cxx
  ${moc_srs}
  ${ui_srcs})
target_link_libraries(QtClassroom ${QT_LIBRARIES} ${RTAUDIO_LIBRARY}
  ${Boost_LIBRARIES})
