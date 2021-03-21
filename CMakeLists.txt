 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq VERSION 1.0.0 DESCRIPTION "lydaq base library")
if(NOT UNIX AND NOT APPLE)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Laurent Mirabito")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libzmq3-dev,libcpprest-dev,libftdi-dev,liblog4cxx-dev,libboost-system-dev,libmongoc-dev")
endif()
set(CPACK_PACKAGE_NAME "lydaq")
set(CPACK_PACKAGE_VENDOR "ip2i")
set(CPACK_VERBATIM_VARIABLES true)
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_SET_DESTDIR true)
set(CPACK_INSTALL_PREFIX /usr/local/lydaq)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(CPACK_SYSTEM_NAME "Linux-arm")
  set(CPACK_TOPLEVEL_TAG "Linux-arm")
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(CPACK_SYSTEM_NAME "Linux-x86_64")
  set(CPACK_TOPLEVEL_TAG "Linux-x86_64")
endif()






#set(FAKE_TARGET fakeTarget)
#if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/netlink/config.h )
#  add_custom_target(fakeTarget  echo "netLink already installed")
#else()
#  if (NOT APPLE AND UNIX)
#    add_custom_target(fakeTarget  ${CMAKE_CURRENT_SOURCE_DIR}/extras/install_netlink)
#  else()
#    add_custom_target(fakeTarget  echo "netLink not yet installed")
#  endif()
#endif()

include(CMake.defs)

message(${CMAKE_SYSTEM_PROCESSOR})
add_subdirectory(drivers)
add_subdirectory(apps)
add_subdirectory(monitoring)
#add_subdirectory(evb)
#add_subdirectory(plugins/demo)
#if (NOT APPLE)
#  add_subdirectory(plugins/zup)
#  add_subdirectory(plugins/genesys)
#  add_subdirectory(plugins/wiener)
#endif()
#add_subdirectory(plugins/grStore)
#add_subdirectory(plugins/mgStore)
#if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
#  set(CPACK_SYSTEM_NAME "Linux-arm")
#  set(CPACK_TOPLEVEL_TAG "Linux-arm")
#  add_subdirectory(plugins/bmp)
#  add_subdirectory(plugins/hih)
#endif()

#if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
#  set(CPACK_SYSTEM_NAME "Linux-x86_64")
#  set(CPACK_TOPLEVEL_TAG "Linux-x86_64")
#  if (NOT APPLE)
#    add_subdirectory(plugins/syx27)
#  endif()
#endif()
#add_subdirectory(lyon/db)
#add_subdirectory(lyon/mpi)
#add_subdirectory(lyon/febv1)
#add_subdirectory(lyon/mbmdcc)
#add_subdirectory(lyon/gricv0)
#add_subdirectory(lyon/gricv1)
#add_subdirectory(lyon/pmr)
#add_subdirectory(lyon/mdcc)

include(CPack)

set(MGROC_EXE
  ${CMAKE_CURRENT_SOURCE_DIR}/apps/mongoroc/mgroc
  )
set(MGROC_PYTHON
  ${CMAKE_CURRENT_SOURCE_DIR}/apps/mongoroc/MongoRoc.py
  ${CMAKE_CURRENT_SOURCE_DIR}/apps/mongoroc/prettyjson.py
  )

file(GLOB RUNCTRL_PYTHON
     "${CMAKE_CURRENT_SOURCE_DIR}/pycontrol/*.py"
     )
set(RUNCTRL_EXE
  ${CMAKE_CURRENT_SOURCE_DIR}/pycontrol/combdaq
  ${CMAKE_CURRENT_SOURCE_DIR}/pycontrol/febdaq
  ${CMAKE_CURRENT_SOURCE_DIR}/pycontrol/difdaq
  )

file(GLOB MONITOR_PYTHON
     "${CMAKE_CURRENT_SOURCE_DIR}/monitoring/scripts/*.py"
     )
set(MONITOR_EXE
  ${CMAKE_CURRENT_SOURCE_DIR}/monitoring/scripts/mgslow
  ${CMAKE_CURRENT_SOURCE_DIR}/monitoring/scripts/slaccess
  )



install(FILES ${MGROC_PYTHON} ${RUNCTRL_PYTHON} ${MONITOR_PYTHON}  DESTINATION share )
install(FILES ${MGROC_EXE} ${RUNCTRL_EXE} ${MONITOR_EXE} DESTINATION bin )

install(CODE "execute_process(COMMAND chmod -R +x ${CMAKE_INSTALL_PREFIX}/bin/)")
