include(CheckCXXSourceCompiles)

function(add_sanitizers TARGET)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
        # Check if ASan actually links (not all GCC installs ship it)
        set(CMAKE_REQUIRED_FLAGS "-fsanitize=address -fno-sanitize=vptr")
        check_cxx_source_compiles("int main() { return 0; }" HAS_ASAN_LINK)
        set(CMAKE_REQUIRED_FLAGS "")

        if(HAS_ASAN_LINK)
            target_compile_options(${TARGET} PRIVATE
                -fsanitize=address,undefined
                -fno-sanitize=vptr
            )
            target_link_options(${TARGET} PRIVATE
                -fsanitize=address,undefined
                -fno-sanitize=vptr
            )
        else()
            message(STATUS "Sanitizers not linkable for ${TARGET} — skipping")
        endif()
    endif()
endfunction()
