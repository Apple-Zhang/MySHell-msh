aux_source_directory(. DIR_LIB_SRCS)

# Add the library
include_directories(../include)
link_directories(../lib)

# Create link library
add_library(mshf ${DIR_LIB_SRCS})

# Add libreadline.so
target_link_libraries(mshf -lreadline)