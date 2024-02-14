project (connector)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

add_executable (Connector "src/connector.c")
add_dependencies (Connector echo_idl_files echo_connector_edl_files)

#set_target_properties (Connector PROPERTIES LINK_FLAGS "-Ttext 0x00800000")
