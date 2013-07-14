cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
project(QtClassroom)

find_package(Qt4 4.8 REQUIRED QtCore QtGui QtMultimedia)
include(${QT_USE_FILE})

qt4_wrap_cpp(moc_srs
  qcMainWindow.h
)

qt4_wrap_ui(ui_srcs
  qcMainWindow.ui
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(QtClassroom MACOSX_BUNDLE
  qcMainWindow.h
  qcMainWindow.cxx
  ${moc_srs}
  ${ui_srcs})
target_link_libraries(QtClassroom ${QT_LIBRARIES})
