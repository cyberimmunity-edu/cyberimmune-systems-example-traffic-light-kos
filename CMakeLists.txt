cmake_minimum_required (VERSION 3.12)

project (echo)

# Initialize CMake library for KasperskyOS SDK.
include (platform)
initialize_platform (FORCE_STATIC)

# Add functions for installing artefacts.
include (platform/install)

# Add project tests.
include (platform/tests)
add_project_tests_main_target ()


# Add Doxygen documentation
# include (platform/doxygen)
# add_project_documentation_main_target ()

# Platform definitions.
include (platform/options)

# Tools for using NK parser.
include (platform/nk)

# Add functions for installing artefacts.
include (platform/install)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

# Add package for working with the virtual file system.
find_package (vfs REQUIRED)
include_directories (${vfs_INCLUDE})

# Add a package with the dhcpcd program implementation.
find_package (rump REQUIRED COMPONENTS DHCPCD_ENTITY)
include_directories (${rump_INCLUDE})

# Add package with VFS program implementations.
find_package (precompiled_vfs REQUIRED)
include_directories (${precompiled_vfs_INCLUDE})

# Choose configuration files and the network configuration mode.
# If none of the options are selected, the ramfs device and the dynamic
# network configuration with fallback to static mode are used.
option (DHCPCD_FALLBACK "Configuration by /etc/dhcpcd.conf"           OFF)
option (DHCPCD_DYNAMIC  "Configuration by /etc/dhcpcd_dynamic.config" OFF)
option (DHCPCD_STATIC   "Configuration by /etc/dhcpcd_static.config"  OFF)

# Attach the directory with the generated config header files.
include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (RESOURCES ${CMAKE_SOURCE_DIR}/resources)



# Recommended flags against vulnerabilities.
# Recommended compiler flags against vulnerabilities.
set(VULN_LDFLAGS "\
    -Wl,-z,noexecstack \
    -Wl,-z,separate-code \
    -Wl,-z,now")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -Wall -Wextra -Wconversion \
    -fPIE -pie \
    -fstack-protector-strong -fstack-clash-protection \
    -mbranch-protection=standard \
    -Wsign-conversion -Wformat=2 -Wformat-security -Werror=format-security \
    -fsanitize=undefined -fsanitize-undefined-trap-on-error")

nk_build_idl_files (echo_idl_files NK_MODULE "echo" IDL "${RESOURCES}/edl/Ping.idl")
nk_build_cdl_files (echo_cdl_files IDL_TARGET echo_idl_files NK_MODULE "echo" CDL "${RESOURCES}/edl/Ping.cdl")
nk_build_edl_files (echo_server_edl_files CDL_TARGET echo_cdl_files NK_MODULE "echo" EDL "${RESOURCES}/edl/Server.edl")
nk_build_edl_files (echo_client_edl_files NK_MODULE "echo" EDL "${RESOURCES}/edl/Client.edl")
nk_build_edl_files (echo_client2_edl_files NK_MODULE "echo" EDL "${RESOURCES}/edl/Client2.edl")
nk_build_edl_files (echo_connector_edl_files NK_MODULE "echo" EDL "${RESOURCES}/edl/Connector.edl")

add_subdirectory (client)
add_subdirectory (client2)
add_subdirectory (connector)
add_subdirectory (json-parser)
add_subdirectory (server)
add_subdirectory (einit)
