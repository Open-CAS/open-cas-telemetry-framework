# Package configuration file.
# This file should be installed in the same directory as octf exported targets

# This file is to be installed in directory, where cmake's
# find_package looks for packages. This allows client build systems
# to call find_package(octf ...) to add octf library into their build.

# Include adjecent file with exported targets
include(${CMAKE_CURRENT_LIST_DIR}/octf.cmake)

# Include adjecent file with exported variables
include(${CMAKE_CURRENT_LIST_DIR}/octf-vars.cmake)