cmake_minimum_required (VERSION 3.0)

add_library (local STATIC
    print.cpp
    print.hpp
    progress.cpp
    progress.hpp
)
set_property (TARGET local PROPERTY CXX_STANDARD 11)
set_property (TARGET local PROPERTY CXX_STANDARD_REQUIRED Yes)
target_include_directories (local PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (local PUBLIC boost)

if (MSVC)
    # Silence some of the microsoft compiler's less useful warnings.
    target_compile_definitions (local PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_options (local PRIVATE /wd4996)

    # Appease VS2015 Update 2. It issues an error which says "You've instantiated 
    # std::atomic<T> with sizeof(T) equal to 2/4/8 and alignof(T) < sizeof(T)"
    target_compile_definitions (local PUBLIC -D_ENABLE_ATOMIC_ALIGNMENT_FIX)
endif ()

#eof CMakeLists.txt
