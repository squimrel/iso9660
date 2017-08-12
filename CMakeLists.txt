cmake_minimum_required(VERSION 3.0)
# Don't confuse it with the libcdio-devel which provides a libiso9660.so.
project(iso9660io)

if(NOT DEFINED BINDIR)
  set(BINDIR bin)
endif()

if(MINGW)
  if(NOT DEFINED LIBDIR)
    set(LIBDIR lib)
  endif()
  if(NOT DEFINED DATADIR)
    set(DATADIR ${LIBDIR})
  endif()
else()
  if(NOT DEFINED LIBDIR)
    # Provides CMAKE_INSTALL_LIBDIR.
    include(GNUInstallDirs)
    set(LIBDIR ${CMAKE_INSTALL_LIBDIR})
  endif()
  if(NOT DEFINED DATADIR)
    set(DATADIR share)
  endif()
endif()

execute_process(COMMAND sh ${CMAKE_SOURCE_DIR}/scripts/version.sh
  OUTPUT_VARIABLE VERSION)
execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/description.txt
  OUTPUT_VARIABLE DESCRIPTION)
set(EXPORT_HEADER include/iso9660.h)
set(PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.h)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3 -DNDEBUG -fvisibility=hidden")

add_custom_command(POST_BUILD
  OUTPUT ${PUBLIC_HEADER}
  COMMAND sh scripts/make_header.sh ARGS ${EXPORT_HEADER} ${PUBLIC_HEADER}
  DEPENDS ${EXPORT_HEADER}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Build library.
include_directories(.)
file(GLOB FILES src/*.cc)
add_library(${CMAKE_PROJECT_NAME} SHARED ${FILES} ${PUBLIC_HEADER})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
  VERSION ${VERSION}
  SOVERSION 0
  PUBLIC_HEADER ${PUBLIC_HEADER})

install(TARGETS ${CMAKE_PROJECT_NAME}
  EXPORT ${CMAKE_PROJECT_NAME}
  # .so
  LIBRARY DESTINATION ${LIBDIR}
  # .dll
  RUNTIME DESTINATION ${BINDIR}
  # .dll.a
  ARCHIVE DESTINATION ${LIBDIR}
  PUBLIC_HEADER DESTINATION include)

configure_file(${CMAKE_PROJECT_NAME}.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc
  DESTINATION ${DATADIR}/pkgconfig)

# Build example.
add_executable(persistent-storage EXCLUDE_FROM_ALL example/persistent-storage.cc example/persistent-storage.h example/file-manipulation.h example/file-manipulation.cc)
target_link_libraries(persistent-storage ${CMAKE_PROJECT_NAME})
