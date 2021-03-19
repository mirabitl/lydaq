 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq_drivers VERSION 1.0.0 DESCRIPTION "Lyon zdaq drivers library")

include(../CMake.defs)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(dirpc Bmp;common;Dif;FebInj;Ftdi;Genesys;GPIO;hih8000;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
  set(dirpc_code Bmp;Dif;FebInj;Ftdi;Genesys;GPIO;hih8000;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(dirpc Caen;common;Dif;Ftdi;Genesys;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
  set(dirpc_code Caen;Dif;Ftdi;Genesys;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
endif()

foreach ( sd IN LISTS dirpc)
  file(GLOB armshead ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/include/*.hh )
  foreach(x IN LISTS armshead)
    set(PC_HEADERS ${PC_HEADERS}  ${x})
  endforeach()
  file(GLOB armshead ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/include )
  foreach(x IN LISTS armshead)
    set(PC_INCLUDEDIRS ${PC_INCLUDEDIRS}  ${x})
  endforeach()
endforeach()
foreach ( sd IN LISTS dirpc_code)
    file(GLOB armshead ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/src/*.cc )
  foreach(x IN LISTS armshead)
    set(PC_SRCS ${PC_SRCS}  ${x})
  endforeach()
  file(GLOB armshead ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/src/*.cxx )
  foreach(x IN LISTS armshead)
    set(PC_SRCS ${PC_SRCS}  ${x})
  endforeach()
endforeach()


foreach(x IN LISTS PC_INCLUDEDIRS )
    message("Header= "${x})
endforeach()



#message(${PC_HEADERS})
#message(${PC_SRCS})
#message(${PC_INCLUDEDIRS})
add_library(lydaq_drivers SHARED ${PC_SRCS} ${PC_HEADERS})
target_include_directories(lydaq_drivers PRIVATE /usr/local/zdaq/include  ${PC_INCLUDEDIRS} )
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  target_link_libraries(lydaq_drivers PRIVATE wiringPi netLink ${ZDAQ_LIBS})
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  target_link_libraries(lydaq_drivers PRIVATE caenhvwrapper netLink ${ZDAQ_LIBS})


endif()

include(CPack)

install(FILES ${PC_HEADERS}  DESTINATION include )
install(TARGETS lydaq_drivers  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)



