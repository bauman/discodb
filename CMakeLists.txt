cmake_minimum_required(VERSION 3.3)
project(libdiscodb)

# unclear whether SuperFastHash in ddb_hash.h is fair use within LGPL
# a less restrictive 32bit hash function is now default hash function
# define LGPL_IS_OK to use the original LGPL'd hashing function
#                   undefined uses the less restrictive implementation
# add_definitions(-DLGPL_IS_OK)

INCLUDE_DIRECTORIES(/usr/include/  src/ )
LINK_DIRECTORIES(/usr/lib64/)
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

enable_testing()

add_executable(tddb_from_kv src/tests/create_and_query.c)
target_link_libraries(tddb_from_kv discodb )
add_test(test2_ddb_from_kv tddb_from_kv disco.out ../src/tests/input.txt)


add_executable(tddb_merge src/tests/create_and_merge.c)
target_link_libraries(tddb_merge discodb )
add_test(test3_ddb_from_ddbs tddb_merge disco_merged.out disco.out)


add_test(test10_ddbs_from_merge ddb_merge /tmp/merged.ddb /tmp/animals.ddb /tmp/myths.ddb)
add_test(test11_ddbs_from_merge_inverted ddb_invert /tmp/inverted_myths.ddb /tmp/myths.ddb)


