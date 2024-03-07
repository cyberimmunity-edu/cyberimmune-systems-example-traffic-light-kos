project (CommunicationSystem)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

add_executable (CommunicationSystem "src/communication_system.c")
add_dependencies (CommunicationSystem traffic_light_idl_files traffic_light_communication_system_edl_files)