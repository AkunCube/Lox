add_library(lox_lib
    chunk.c
    compiler.c
    debug.c
    memory.c
    object.c
    scanner.c
    table.c
    value.c
    vm.c
)

add_executable(clox main.c)
target_link_libraries(clox lox_lib)
