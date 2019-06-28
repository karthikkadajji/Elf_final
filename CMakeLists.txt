cmake_minimum_required(VERSION 3.6)
include(ExternalProject)
project(Elf_cleaned)
set(CMAKE_CXX_STANDARD 11)

ExternalProject_Add(bitweaving
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/bitweaving-lib
	INSTALL_DIR ${PROJECT_SOURCE_DIR}/bitweaving-lib
        BUILD_IN_SOURCE TRUE
        CONFIGURE_COMMAND ""
	BUILD_COMMAND make
        INSTALL_COMMAND "")

include_directories(bitweaving-lib/include)
find_package(Boost 1.63 COMPONENTS system chrono)

set(SOURCE_FILES
        SortingEngine/introSort.h
        SortingEngine/radixSort.h
        SortingEngine/sortingEngine.h
        SortingEngine/sortingEngineUtils.h
        Utils/configs.h
        Utils/structures.h
        benchmark.h
        bitweaving.cpp
        bitweaving.h
        colseqscan.h
        Config.h
        Elf_final64.h
        Elf_final64_subtree.h
        Elf_final64_nodelevel_enqueue.h
        Imprint.cpp
        Imprint.h
        ImprintSIMD64WithoutCompression.h
        ImprintSIMDWithoutCompression.h
        ImprintWithoutCompression.h
        Index.h
        main.cpp
        MyKDTreeSmall.h
        seqscan.h
        simdseqscan.h
        Store.h
        tpc_h.h
        Util.h
        SafeQueue.h
        ThreadPool.h
        )

add_executable(Elf_cleaned ${SOURCE_FILES})
add_dependencies(Elf_cleaned bitweaving)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(Elf_cleaned ${Boost_LIBRARIES})
endif()
target_link_libraries(Elf_cleaned ${PROJECT_SOURCE_DIR}/bitweaving-lib/libbitweaving.a)
