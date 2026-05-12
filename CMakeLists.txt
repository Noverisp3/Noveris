cmake_minimum_required(VERSION 3.10)
project(Noveris VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCE_FILES
    src/Lexer.cpp
    src/Parser.cpp
    src/Interpreter.cpp
    src/main.cpp
)

# Header files
set(HEADER_FILES
    include/AST.h
    include/Lexer.h
    include/Parser.h
    include/Interpreter.h
)

# Create executable
add_executable(noveris ${SOURCE_FILES} ${HEADER_FILES})

# Set output directory
set_target_properties(noveris PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable warnings
if(MSVC)
    target_compile_options(noveris PRIVATE /W4)
else()
    target_compile_options(noveris PRIVATE -Wall -Wextra -pedantic)
endif()

# Static linking for MSYS2/MinGW
if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static")
    target_link_options(noveris PRIVATE -static)
    target_link_libraries(noveris PRIVATE -static-libgcc -static-libstdc++)
endif()
