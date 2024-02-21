project (Connection)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

add_executable (Connection  "src/connection.c")
add_dependencies (Connection traffic_light_idl_files traffic_light_connection_edl_files)

#set_target_properties (Client PROPERTIES LINK_FLAGS "-Ttext 0x00800000")
