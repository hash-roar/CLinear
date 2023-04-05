aux_source_directory( . DIR_SRCS )

# add gtest libary with conan

include_directories(${PROJECT_SOURCE_DIR}/core/base)
include_directories(${PROJECT_SOURCE_DIR}/core/utils)
find_package(GTest REQUIRED)
find_package(fmt)
# build all tests
foreach( testsourcefile ${DIR_SRCS} )
    # I used a simple string replace, to cut off .cpp.
    string( REPLACE ".cc" "" testname ${testsourcefile} )
    get_filename_component(testname ${testname} NAME)
    add_executable( ${testname} ${testsourcefile} )
    target_link_libraries(${testname} GTest::GTest GTest::Main)
    target_link_libraries(${testname} fmt::fmt)
endforeach( testsourcefile ${DIR_SRCS} )
