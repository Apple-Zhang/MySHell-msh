# CMakeLists for Myshell
# not including mls, mcp.
cmake_minimum_required(VERSION 3.16.0)
project(Myshell)

# Add all source code in ./src
aux_source_directory(./src DIR_SRCS)

# Add header
add_subdirectory(./src/msh_lib)

# Add executable file
add_executable(msh ./src/main_msh.c)

# Add linker
target_link_libraries(msh mshf)