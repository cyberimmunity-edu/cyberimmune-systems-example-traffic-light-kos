project (DiagnosticSystem)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

add_executable (DiagnosticSystem "src/diagnostic_system.c")
add_dependencies (DiagnosticSystem traffic_light_diagnostic_system_edl_files)