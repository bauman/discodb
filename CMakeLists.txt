cmake_minimum_required(VERSION 3.23)
project(libdiscodb)

# unclear whether SuperFastHash in ddb_hash.h is fair use within LGPL
# a less restrictive 32bit hash function is now default hash function
# define LGPL_IS_OK to use the original LGPL'd hashing function
#                   undefined uses the less restrictive implementation
# add_definitions(-DLGPL_IS_OK)


##  SECTION 1 -- COMPILE Checks
INCLUDE(CheckIncludeFiles)
INCLUDE (CheckLibraryExists)

option(lgpl-is-ok "use SuperFastHash rather than CityHash" OFF)


if (APPLE)
    set(HOMEBREW_PREFIX "/opt/homebrew"
            CACHE PATH "Path to Homebrew installation")
    set(CMAKE_PREFIX_PATH
            "${HOMEBREW_PREFIX}")

    INCLUDE_DIRECTORIES(${CMAKE_PREFIX_PATH}/include/  src/ )
    LINK_DIRECTORIES(${HOMEBREW_PREFIX}/lib)
else()
    INCLUDE_DIRECTORIES(/usr/include/  src/ )
    LINK_DIRECTORIES(/usr/lib64/)

    CHECK_INCLUDE_FILES(cmph.h HAVE_CMPH_H)
    if(NOT HAVE_CMPH_H)
        message(SEND_ERROR "CMPH header is required, stopping")
        return()
    endif()
    CHECK_LIBRARY_EXISTS(cmph cmph_config_new "" HAVE_CMPH_LIB)
    if(NOT HAVE_CMPH_LIB)
        message(SEND_ERROR "cmph library is required, stopping")
        return()
    endif()
endif ()

if(lgpl-is-ok)
    message("-- requested SuperFastHash with -D lgpl-is-ok, will build with SuperFastHash rather than CityHash")
    add_definitions(-DLGPL_IS_OK)
endif()


set(SOURCE_FILES    src/ddb_delta.c
                    src/ddb_deltalist.c
                    src/ddb_membuffer.c
                    src/ddb_queue.c
                    src/ddb.c
                    src/ddb_cmph.c
                    src/ddb_cnf.c
                    src/ddb_cons.c
                    src/ddb_huffman.c
                    src/ddb_list.c
                    src/ddb_map.c
                    src/ddb_view.c)

set(HEADER_FILES    src/ddb_bits.h
        src/ddb_cmph.h
        src/ddb_delta.h
        src/ddb_deltalist.h
        src/ddb_hash.h
        src/ddb_huffman.h
        src/ddb_internal.h
        src/ddb_list.h
        src/ddb_map.h
        src/ddb_membuffer.h
        src/ddb_profile.h
        src/ddb_queue.h
        src/ddb_types.h
        src/discodb.h
        )


add_library(discodb SHARED ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(discodb cmph)

#gcc -Wall -O2  -shared -o libdiscodb.so -fPIC  *.c -lcmph



add_executable(ddb_create src/util/create.c)
target_link_libraries(ddb_create discodb )

add_executable(ddb_query src/util/query.c)
target_link_libraries(ddb_query discodb )

add_executable(ddb_merge src/util/merge.c)
target_link_libraries(ddb_merge discodb )

add_executable(ddb_invert src/util/invert.c)
target_link_libraries(ddb_invert discodb )


install(FILES ${HEADER_FILES} DESTINATION include)
install(TARGETS discodb ddb_create ddb_query ddb_merge ddb_invert
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

enable_testing()

add_executable(tddb_from_kv src/tests/create_and_query.c)
target_link_libraries(tddb_from_kv discodb )
add_test(test2_ddb_from_kv tddb_from_kv disco.out ../src/tests/input.txt)


add_executable(tddb_merge src/tests/create_and_merge.c)
target_link_libraries(tddb_merge discodb )
add_test(test3_ddb_from_ddbs tddb_merge disco_merged.out disco.out)


add_test(test10_ddbs_from_merge ddb_merge /tmp/merged.ddb /tmp/animals.ddb /tmp/myths.ddb)
add_test(test11_ddbs_from_merge_inverted ddb_invert /tmp/inverted_myths.ddb /tmp/myths.ddb)


