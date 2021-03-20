 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq_drivers VERSION 1.0.0 DESCRIPTION "Lyon zdaq drivers library")

include(../CMake.defs)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(dirpc Bmp;common;Dif;FebInj;Ftdi;Genesys;GPIO;hih8000;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
  set(dirapp Builder;C3I;C4I;Control;Dif;DIF;FebCms;FEBV1;Genesys;Gric;GRICV0;MBMDCC;Mdcc;MdccEcal;monitoring;N1470;PMR;Sdcc;Wiener;Zup;Bmp;hih8000;FebInj;GPIO)

endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(dirpc Caen;common;Dif;Ftdi;Genesys;Mdcc;MdccEcal;MgRoc;MPI;N1470;Pmr;Sdcc;Wiener;Zup)
  #set(dirapp Caen;Builder;C3I;C4I;Control;Dif;DIF;FebCms;FEBV1;Genesys;Gric;GRICV0;MBMDCC;Mdcc;MdccEcal;N1470;PMR;Sdcc;Wiener;Zup)
  set(dirapp Builder;Caen;C3I;C4I;Control;Dif;DIF;FebCms;FEBV1;Genesys;Gric;GRICV0;MBMDCC;Mdcc;MdccEcal;monitoring;N1470;PMR;Sdcc;Wiener;Zup)
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
    message("Header= "${x})
endforeach()



#message(${PC_HEADERS})
#message(${PC_SRCS})
#message(${PC_INCLUDEDIRS})
#set(header_Caen Caen/include/LCaenServer.hh  )
#set(files_Caen Caen/src/LCaenDaemon.cxx Caen/src/LCaenServer.cxx  )

#set(header_C3I C3I/include/C3iManager.hh C3I/include/C3iMpi.hh C3I/include/MpiInterface.hh )
#set(files_C3I  C3I/src/C3iDaemon.cxx C3I/src/C3iManager.cc C3I/src/C3iMpi.cc C3I/src/MpiInterface.cc  )

set(files_Builder Builder/include/Builder.hh Builder/src/Builder.cxx Builder/src/BuilderDaemon.cxx  )

set(files_C3I C3I/include/C3iManager.hh C3I/include/C3iMpi.hh C3I/include/MpiInterface.hh C3I/src/C3iDaemon.cxx C3I/src/C3iManager.cc C3I/src/C3iMpi.cc C3I/src/MpiInterface.cc  )

set(files_C4I C4I/include/C4iInterface.hh C4I/include/C4iManager.hh C4I/src/board.cc C4I/src/C4iDaemon.cxx C4I/src/C4iManager.cc C4I/src/datahandler.cc C4I/src/interface.cc C4I/src/registerhandler.cc C4I/src/slchandler.cc C4I/src/sockethandler.cc  )

set(files_Caen Caen/include/LCaenServer.hh Caen/src/LCaenDaemon.cxx Caen/src/LCaenServer.cxx  )

set(files_Control Control/include/FullDaq.hh Control/src/FDaqDaemon.cxx  Control/src/FullDaq.cxx  )

set(files_Dif Dif/include/LDIF.hh Dif/include/LDIFServer.hh Dif/src/LDIF.cxx Dif/src/LDIFDaemon.cxx Dif/src/LDIFServer.cxx  )

set(files_DIF DIF/include/DIFInterface.hh DIF/include/DIFManager.hh DIF/src/DIFDaemon.cxx DIF/src/DIFInterface.cc DIF/src/DIFManager.cc  )

set(files_FebCms FebCms/include/debug.hh FebCms/include/MessageHandler.hh FebCms/include/TdcWiznet.hh FebCms/include/WiznetInterface.hh FebCms/include/WiznetManager.hh FebCms/include/WiznetMessageHandler.hh FebCms/include/WiznetTest.hh FebCms/src/FebDaemon.cxx FebCms/src/MessageHandler.cc FebCms/src/TdcWiznet.cc  FebCms/src/WiznetInterface.cc FebCms/src/WiznetManager.cc FebCms/src/WiznetMessageHandler.cc FebCms/src/WiznetTest.cc  )

set(files_FEBV1 FEBV1/include/Febv1Interface.hh FEBV1/include/Febv1Manager.hh FEBV1/src/board.cc FEBV1/src/datahandler.cc FEBV1/src/Febv1Daemon.cxx FEBV1/src/Febv1Manager.cc FEBV1/src/interface.cc FEBV1/src/registerhandler.cc FEBV1/src/sockethandler.cc  )

set(files_Genesys Genesys/include/LGenesysServer.hh Genesys/src/LGenesysDaemon.cxx Genesys/src/LGenesysServer.cxx  )

set(files_Gric Gric/include/debug.hh Gric/include/GricManager.hh Gric/include/GricMpi.hh Gric/include/MessageHandler.hh Gric/include/MpiInterface.hh Gric/include/MpiMessageHandler.hh Gric/src/GricDaemon.cxx Gric/src/GricManager.cc Gric/src/GricMpi.cc Gric/src/MessageHandler.cc Gric/src/MpiInterface.cc Gric/src/MpiMessageHandler.cc  )

set(files_GRICV0 GRICV0/include/Gricv0Interface.hh GRICV0/include/Gricv0Manager.hh GRICV0/src/board.cc GRICV0/src/datahandler.cc GRICV0/src/Gricv0Daemon.cxx GRICV0/src/Gricv0Manager.cc GRICV0/src/interface.cc GRICV0/src/registerhandler.cc GRICV0/src/sensorhandler.cc GRICV0/src/sockethandler.cc  )

set(files_MBMDCC MBMDCC/include/MBMDCCInterface.hh MBMDCC/include/MBMDCCManager.hh MBMDCC/src/board.cc MBMDCC/src/interface.cc MBMDCC/src/MBMDCCDaemon.cxx MBMDCC/src/MBMDCCManager.cc MBMDCC/src/registerhandler.cc MBMDCC/src/sockethandler.cc  )

set(files_Mdcc Mdcc/include/LMdccServer.hh Mdcc/src/LMdccDaemon.cxx Mdcc/src/LMdccServer.cxx  )

set(files_MdccEcal MdccEcal/include/LMdccEcalServer.hh MdccEcal/src/LMdccEcalDaemon.cxx MdccEcal/src/LMdccEcalServer.cxx  )

set(files_monitoring monitoring/include/monitorSubscriber.hh monitoring/src/monitorDaemon.cxx monitoring/src/monitorSubscriber.cxx  )

set(files_N1470 N1470/include/LN1470Server.hh N1470/src/LN1470Daemon.cxx N1470/src/LN1470Server.cxx  )

set(files_PMR PMR/include/PMRInterface.hh PMR/include/PMRManager.hh PMR/src/PMRDaemon.cxx PMR/src/PMRInterface.cc PMR/src/PMRManager.cc  )

set(files_Sdcc Sdcc/include/LCccServer.hh Sdcc/src/LCccDaemon.cxx Sdcc/src/LCccServer.cxx  )

set(files_Wiener Wiener/include/LWienerServer.hh Wiener/src/LWienerDaemon.cxx Wiener/src/LWienerServer.cxx  )

set(files_Zup Zup/include/LZupServer.hh Zup/src/LZupDaemon.cxx Zup/src/LZupServer.cxx  )

set(files_Bmp Bmp/include/LBmpServer.hh Bmp/src/LBmpDaemon.cxx Bmp/src/LBmpServer.cxx  )

set(files_hih8000 hih8000/include/LHih8000Server.hh hih8000/src/LHih8000Daemon.cxx hih8000/src/LHih8000Server.cxx  )

set(files_FebInj FebInj/include/FebInjServer.hh FebInj/src/FebInjDaemon.cxx FebInj/src/FebInjServer.cxx  )

set(files_GPIO GPIO/include/LGpioServer.hh GPIO/src/LGpioDaemon.cxx GPIO/src/LGpioServer.cxx  )


#message(${files_Caen})
#message(${files_Builder})
link_directories( /usr/local/zdaq/lib)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  foreach ( sd IN LISTS dirapp)
    add_executable(ex_${sd}  ${files_${sd}} )
    target_link_libraries(ex_${sd} PRIVATE  zdaq sqlite3 ftdi pthread netLink lydaq_drivers ${ZDAQ_LIBS})
    target_include_directories(ex_${sd} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/include /usr/local/zdaq/include  ${PC_INCLUDEDIRS} )
    set(EX_BINS ${EX_BINS} ex_${sd})
  endforeach()
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  foreach ( sd IN LISTS dirapp)
    add_executable(ex_${sd}  ${files_${sd}} )
    target_link_libraries(ex_${sd} PRIVATE zdaq sqlite3 ftdi pthread caenhvwrapper netLink lydaq_drivers ${ZDAQ_LIBS})
    target_include_directories(ex_${sd} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${sd}/include /usr/local/zdaq/include  ${PC_INCLUDEDIRS} )
    set(EX_BINS ${EX_BINS} ex_${sd})
  endforeach()

endif()

#include(CPack)

install(FILES ${PC_HEADERS}  DESTINATION include )
install(TARGETS ${EX_BINS}  DESTINATION bin ARCHIVE DESTINATION bin)



