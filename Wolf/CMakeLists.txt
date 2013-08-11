cmake_minimum_required(VERSION 2.4)

INCLUDE(FindSDL)
INCLUDE(FindSDL_net)
INCLUDE(FindSDL_image)
INCLUDE(FindOpenGL)

FILE(GLOB source
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(wolf ${source})

target_link_libraries(wolf
	${SDL_LIBRARY}
	${SDL_NET_LIBRARY}
	${SDL_IMAGE_LIBRARY}
	${OPENGL_LIBRARY}
)

include_directories(wolf
	${SDL_INCLUDE_DIR}
)

