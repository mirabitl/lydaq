 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq_monitor VERSION 1.0.0 DESCRIPTION "Lyon monitor plugins library")

include(../CMake.defs)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(dirpc Bmp;common;Dif;FebInj;Ftdi;Genesys;GPIO;hih8000;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
  set(dirapp mgstore;grstore;genesys;wiener;zup;bmp;hih)

endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(dirpc Caen;common;Dif;Ftdi;Genesys;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
  #set(dirapp Caen;Builder;C3I;C4I;Control;Dif;DIF;FebCms;FEBV1;Genesys;Gric;GRICV0;MBMDCC;Mdcc;MdccEcal;N1470;PMR;Sdcc;Wiener;Zup)
    set(dirapp mgstore;grstore;genesys;wiener;zup;syx27)
endif()

foreach ( sd IN LISTS dirpc)
  file(GLOB armshead ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/${sd}/include/*.hh )
  foreach(x IN LISTS armshead)
    set(PC_HEADERS ${PC_HEADERS}  ${x})
  endforeach()
  file(GLOB armshead ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/${sd}/include )
  foreach(x IN LISTS armshead)
    set(PC_INCLUDEDIRS ${PC_INCLUDEDIRS}  ${x})
  endforeach()
endforeach()


foreach(x IN LISTS PC_INCLUDEDIRS )
    message( "Header= " ${x} )
endforeach()



#message(${PC_HEADERS})
#message(${PC_SRCS})
#message(${PC_INCLUDEDIRS})
#set(header_Caen Caen/include/LCaenServer.hh  )
#set(files_Caen Caen/src/LCaenDaemon.cxx Caen/src/LCaenServer.cxx  )

#set(header_C3I C3I/include/C3iManager.hh C3I/include/C3iMpi.hh C3I/include/MpiInterface.hh )
#set(files_C3I  C3I/src/C3iDaemon.cxx C3I/src/C3iManager.cc C3I/src/C3iMpi.cc C3I/src/MpiInterface.cc  )
set(files_bmp bmp/include/bmpPlugin.hh bmp/src/bmpPlugin.cxx  )
set(files_genesys genesys/include/genesysPlugin.hh genesys/src/genesysPlugin.cxx )
set(files_grstore grstore/include/graphite-client.h grstore/include/grStore.hh grstore/src/graphite-client.c grstore/src/grStore.cxx  )
set(files_hih hih/include/hihPlugin.hh hih/src/hihPlugin.cxx  )
set(files_mgstore mgstore/include/mgStore.hh mgstore/src/mgStore.cxx  )
set(files_syx27 syx27/include/syx27Plugin.hh syx27/src/syx27Plugin.cxx  )
set(files_wiener wiener/include/wienerPlugin.hh wiener/src/wienerPlugin.cxx  )
set(files_zup zup/include/zupPlugin.hh zup/src/zupPlugin.cxx  )


#message(${files_Caen})
#message(${files_Builder})
link_directories( /usr/local/zdaq/lib)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  foreach ( sd IN LISTS dirapp)
    add_library(plugin_${sd} SHARED ${files_${sd}} )
    target_link_libraries(plugin_${sd} PRIVATE mongoc-1.0 bson-1.0 zdaq sqlite3 ftdi pthread netLink lydaq_drivers ${ZDAQ_LIBS})
    target_include_directories(plugin_${sd} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/include /usr/local/zdaq/include /usr/include/libmongoc-1.0/ /usr/include/libbson-1.0/   ${PC_INCLUDEDIRS} )
    set(PLUGIN_LIBS ${PLUGIN_LIBS} plugin_${sd})
  endforeach()
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  foreach ( sd IN LISTS dirapp)
    add_library(plugin_${sd} SHARED ${files_${sd}} )
    target_link_libraries(plugin_${sd} PRIVATE mongoc-1.0 bson-1.0 zdaq sqlite3 ftdi pthread caenhvwrapper netLink lydaq_drivers ${ZDAQ_LIBS})
    target_include_directories(plugin_${sd} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/include /usr/local/zdaq/include /usr/include/libmongoc-1.0/ /usr/include/libbson-1.0/   ${PC_INCLUDEDIRS} )
    set(PLUGIN_LIBS ${PLUGIN_LIBS} plugin_${sd})
  endforeach()

endif()

#include(CPack)

#install(FILES ${PC_HEADERS}  DESTINATION include )
install(TARGETS ${PLUGIN_LIBS}  DESTINATION lib ARCHIVE DESTINATION lib)
