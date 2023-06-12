add_executable(test_diff test_diff.cpp)
target_link_libraries(test_diff GTest::gtest_main)
target_include_directories(test_diff PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src)

include(GoogleTest)

gtest_discover_tests(test_diff)
