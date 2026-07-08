function(add_compiler_warnings TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /W4 /WX)
    else()
        target_compile_options(${TARGET} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wconversion -Wno-error=conversion -Wno-error=sign-conversion
            -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
            -Woverloaded-virtual
            -Wnull-dereference -Wdouble-promotion
            -Wformat=2
            $<$<CONFIG:Release>:-Werror>
        )
    endif()
endfunction()
