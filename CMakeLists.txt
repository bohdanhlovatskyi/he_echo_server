cmake_minimum_required(VERSION 3.15)

#! Check every comment after the "#!"

#! CHANGE YOUR PROJECT NAME
#  It is used as your project's main executable name. 
set(PROJECT_NAME start_server)
project(${PROJECT_NAME} CXX) # project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_COMPILER clang++)

SET(CLANG_COROUTINES_COMPILE_FLAGS "-Wall -Wextra -std=gnu++2a -fcoroutines-ts -stdlib=libc++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_COROUTINES_COMPILE_FLAGS}")

##########################################################
# User configurable options of the template
##########################################################
# Note: symbols like WARNINGS_AS_ERRORS in configuration are intentionally variables
# and not CMake options --using options creates too much problem for students.

#! It is a good practice to set "WARNINGS_AS_ERRORS" ON,
#  but sometimes it creates too much trouble, so default is OFF.
set(WARNINGS_AS_ERRORS OFF)

#! Always use PVS Studio while developing. 
set(ENABLE_PVS_STUDIO OFF)

#! Select appropriate sanitizers.
#  Definitely enable sanitizers while developing.
#  Disable it for the production builds and before submitting for grading.
#  Only one of Memory (MSAN), Address (ASAN), or Thread (TSan)
#  sanitizers is applicable at the time -- the first defined.

#! UndefinedBehaviorSanitizer (UBSan)
#  Info: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
set(ENABLE_UBSan OFF)
#! AddressSanitizer -- detects use after free or after scope exit,
#  memory overflows and leaks.  
#  Info: https://github.com/google/sanitizers/wiki/AddressSanitizer
set(ENABLE_ASAN OFF)
#! ThreadSanitizer -- detects data races.
set(ENABLE_TSan OFF)
#! MemorySanitizer detects uninitialized memory reads 
#  Info: https://github.com/google/sanitizers/wiki/MemorySanitizer
set(ENABLE_MSAN OFF)


#! Be default -- build release version if not specified otherwise.
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# Warnings as errors should be imported here -- do not move this line
include(cmake/CompilerWarnings.cmake)

##########################################################
# Project files, packages, libraries and so on
##########################################################

# proposed by this issue: https://github.com/boostorg/asio/issues/362
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT=1")

#! Add external packages
# options_parser requires boost::program_options library
find_package(Boost COMPONENTS system thread regex coroutine REQUIRED)
find_package(TBB REQUIRED)

#! Specify targets
set(SERVER_TARGETS
        threaded_epoll
        syncronous
        threaded
        multiprocess
        select
        common_sockets
        boost_syncronous
        boost_threaded
        boost_async
        boost_thread_pool
        coro_boost
        stackful
        stackless
        )

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SERVER_TARGETS
            epoll
            io_submit
            )
endif ()

set(LINK_LIBS_threaded_epoll
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_syncronous
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_threaded
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_multiprocess
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_select
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_common_sockets
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_boost_syncronous
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_boost_threaded
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_boost_async
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_boost_thread_pool
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_coro_boost
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_stackful
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_stackless
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_epoll
        ${BOOST_KEY_WORD}
        )
set(LINK_LIBS_io_submit
        ${BOOST_KEY_WORD}
        )

#! Project executables source compilation
foreach (TARGET ${SERVER_TARGETS})
    MESSAGE("Compiling target: ${TARGET}")
    add_executable(${TARGET} main.cpp
            servers/${TARGET}.cpp
            servers/common_sockets.cpp
            )

    string(TOUPPER ${TARGET} TARGET_SPECIFIER)
    target_compile_definitions(${TARGET} PUBLIC "TARGET_${TARGET_SPECIFIER}")

    target_include_directories(${TARGET} PRIVATE servers)

    # link boost
    target_include_directories(${TARGET} PRIVATE ${Boost_INCLUDE_DIR})
    target_link_libraries(${TARGET} ${Boost_LIBRARIES})

    # link tbb
    target_include_directories(${TARGET} PRIVATE ${TBB_INCLUDE_DIRS})
    target_link_libraries(${TARGET} TBB::tbb)

endforeach ()


##########################################################
# Fixed CMakeLists.txt part
##########################################################

foreach (TARGET ${SERVER_TARGETS})
    INSTALL(TARGETS ${TARGET}
            DESTINATION bin)
endforeach ()

# Define ALL_TARGETS variable to use in PVS and Sanitizers
set(ALL_TARGETS ${SERVER_TARGETS})

# Include CMake setup
include(cmake/main-config.cmake)