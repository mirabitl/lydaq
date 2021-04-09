 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lydaq_rootsdhcal VERSION 0.0.1 DESCRIPTION "Event builder library")
find_package(ROOT)
#add_subdirectory(pns)
set(SRCS
  src/bin23.cc
  )
	
#set(HEADERS
#  include/rootsdhcalProcessor.hh
#	)

add_library(lydaq_rootsdhcal SHARED ${SRCS})


include(../../CMake.defs)


target_link_libraries(lydaq_rootsdhcal PRIVATE lydaq_difaccess ${ROOT_LIBRARIES} )

#target_link_directories(lydaq_rootsdhcal PRIVATE .. )
target_include_directories(lydaq_rootsdhcal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../sdhcal/include /usr/local/zdaq/include  ${ROOT_INCLUDE_DIRS} )





# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lydaq_rootsdhcal  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


