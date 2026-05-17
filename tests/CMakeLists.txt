enable_testing()
include(GoogleTest)

add_executable(
   prodgraphtest
   prodgraphtest.cpp
)

target_link_libraries(
   prodgraphtest
   "factorymodel"
   gtest
   gtest_main
)

gtest_discover_tests(
   prodgraphtest
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
