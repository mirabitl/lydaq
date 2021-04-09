 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq_difaccess VERSION 0.0.1 DESCRIPTION "Event builder library")
find_package(ROOT)
#add_subdirectory(pns)
set(SRCS
  src/SdhcalDifAccess.cc
  )
	
set(HEADERS
  include/SdhcalDifAccess.hh

	)

add_library(lydaq_difaccess SHARED ${SRCS} ${HEADERS})


include(../../CMake.defs)


#target_link_libraries(lydaq_difaccess PRIVATE ${ROOT_LIBRARIES} )

#target_link_directories(evb PRIVATE .. )
#target_include_directories(lydaq_difaccess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include /usr/local/zdaq/include ${ROOT_INCLUDE_DIRS} )

target_include_directories(lydaq_difaccess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include  )





# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lydaq_difaccess  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

file(GLOB INCLUDE_TARGET
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*"
     )   
install(FILES ${INCLUDE_TARGET}  DESTINATION include )


