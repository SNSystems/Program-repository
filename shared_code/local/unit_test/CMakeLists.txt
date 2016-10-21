cmake_minimum_required (VERSION 3.0)

add_library (local_test STATIC
    test_progress.cpp
)

set_property (TARGET local_test PROPERTY CXX_STANDARD 11)
set_property (TARGET local_test PROPERTY CXX_STANDARD_REQUIRED Yes)
target_link_libraries (local_test PRIVATE local gtest gmock)

#eof CMakeLists.txt
