project (client2)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

add_executable (Client2 "src/client.c")
add_dependencies (Client2 echo_idl_files echo_client2_edl_files)

#set_target_properties (Client2 PROPERTIES LINK_FLAGS "-Ttext 0x00800000")
set_target_properties (Client2 PROPERTIES LINK_FLAGS "-no-pie -Ttext 0x600000")