set(SN76489_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(SN76489_SOURCES
	sn76489.cpp)

set(SN76489_HEADERS
	sn76489.h)

add_library(sn76489 STATIC ${SN76489_SOURCES} ${SN76489_HEADERS})
target_include_directories(sn76489 PUBLIC
	${SN76489_INCLUDE_DIR})