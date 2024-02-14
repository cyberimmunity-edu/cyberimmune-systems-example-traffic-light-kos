cmake_minimum_required (VERSION 3.12)

project (echo)

# Initialize CMake library for KasperskyOS SDK.
include (platform)
initialize_platform (FORCE_STATIC)

# Add Doxygen documentation
include (platform/doxygen)
add_project_documentation_main_target ()

# Tools for using NK parser.
include (platform/nk)

nk_build_idl_files (echo_idl_files NK_MODULE "echo" IDL "resources/Ping.idl")
nk_build_cdl_files (echo_cdl_files IDL_TARGET echo_idl_files NK_MODULE "echo" CDL "resources/Ping.cdl")
nk_build_edl_files (echo_server_edl_files CDL_TARGET echo_cdl_files NK_MODULE "echo" EDL "resources/Server.edl")
nk_build_edl_files (echo_client_edl_files NK_MODULE "echo" EDL "resources/Client.edl")
nk_build_edl_files (echo_client2_edl_files NK_MODULE "echo" EDL "resources/Client2.edl")
nk_build_edl_files (echo_connector_edl_files NK_MODULE "echo" EDL "resources/Connector.edl")

add_subdirectory (client)
add_subdirectory (client2)
add_subdirectory (connector)
add_subdirectory (server)
add_subdirectory (einit)
