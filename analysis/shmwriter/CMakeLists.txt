 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq_shmwriter VERSION 0.0.1 DESCRIPTION "Event builder library")
find_package(ROOT)
#add_subdirectory(pns)
set(SRCS
  src/shmwriterProcessor.cxx
  )
	
set(HEADERS
  include/shmwriterProcessor.hh

	)

add_library(lydaq_shmwriter SHARED ${SRCS} ${HEADERS})


include(../../CMake.defs)


target_link_libraries(lydaq_shmwriter PRIVATE ${ROOT_LIBRARIES} )

#target_link_directories(evb PRIVATE .. )
target_include_directories(lydaq_shmwriter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include /usr/local/zdaq/include ${ROOT_INCLUDE_DIRS} )





# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lydaq_shmwriter  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


