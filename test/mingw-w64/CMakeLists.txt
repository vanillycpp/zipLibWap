cmake_minimum_required(VERSION 3.9)


PROJECT (Zip_lib_tst LANGUAGES C CXX)

set (CMAKE_CXX_FLAGS "-O0 -g -Wall -Wno-unused-parameter -std=gnu++11")
# set (CMAKE_EXE_LINKER_FLAGS "-static -static-libstdc++ -static-libgcc -municode")
# set (CMAKE_EXE_LINKER_FLAGS "-lz -municode")

find_package(ZLIB REQUIRED)

# include_directories(${ZLIB_INCLUDE_DIRS})

include_directories(.)
include_directories("../../zipLibInc")

set(SRC_FILES 
  main.cpp
  ../../ZipLibWrap.cpp
)

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_FILES})

#message(${ZLIB_LIBRARIES})
#message(${ZLIB_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libzip.a ${ZLIB_LIBRARIES} libbz2.dll.a) 

# C:/msys32/mingw32/lib/libz.dll.a C:/msys32/mingw32/lib/libz.a