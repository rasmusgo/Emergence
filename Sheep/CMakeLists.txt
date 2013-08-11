cmake_minimum_required(VERSION 2.4)

INCLUDE(FindSDL)
INCLUDE(FindSDL_net)

FILE(GLOB source
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(sheep ${source})

target_link_libraries(sheep
	${SDL_LIBRARY}
	${SDL_NET_LIBRARY}
)

include_directories(sheep
	${SDL_INCLUDE_DIR}
)

