set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(SANITIZERS_FLAGS -fsanitize=address -fsanitize=undefined)
set(DEBUG_FLAGS -ggdb -O0 ${SANITIZERS_FLAGS})

add_compile_options(-Wall -Wextra -Werror -pedantic ${DEBUG_FLAGS})

add_link_options(${SANITIZERS_FLAGS})
