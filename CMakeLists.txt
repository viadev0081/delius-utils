cmake_minimum_required(VERSION 3.16)

project(delius-utils
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# --- ncursesw ---------------------------------------------------------------
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)

# --- change-script ----------------------------------------------------------
add_executable(cs
    change-script/src/chscript.cpp
    change-script/src/lib/lib.cpp
)
target_include_directories(cs PRIVATE change-script/src)
target_link_libraries(cs PRIVATE ${CURSES_LIBRARIES})
target_include_directories(cs PRIVATE ${CURSES_INCLUDE_DIRS})

set_target_properties(cs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/change-script/bin"
)

# --- cash-terminal ----------------------------------------------------------
add_executable(cash_term
    cash-terminal/src/cash_term.cpp
    cash-terminal/src/lib/lib.cpp
)

target_include_directories(cash_term PRIVATE cash-terminal/src)
target_link_libraries(cash_term PRIVATE ${CURSES_LIBRARIES})
target_include_directories(cash_term PRIVATE ${CURSES_INCLUDE_DIRS})

set_target_properties(cash_term PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/cash-terminal/bin"
)

# Ship the runtime data next to the built binary.
add_custom_command(TARGET cash_term POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/cash-terminal/data"
        "$<TARGET_FILE_DIR:cash_term>/data"
    COMMENT "Copying cash-terminal data"
)

# --- fastfetch-logos --------------------------------------------------------
add_executable(ff-logos
    fastfetch-logos/src/main.cpp
)

target_include_directories(cash_term PRIVATE fastfetch-logos/src)
target_link_libraries(ff-logos PRIVATE ${CURSES_LIBRARIES})
target_include_directories(ff-logos PRIVATE ${CURSES_INCLUDE_DIRS})

set_target_properties(ff-logos PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fastfetch-logos/bin"
)
