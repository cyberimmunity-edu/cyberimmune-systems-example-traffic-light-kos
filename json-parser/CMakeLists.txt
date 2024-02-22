project (json_parser)

# Set compilation flags.
project_header_default ("STANDARD_GNU_11:YES" "EXTRA_STRICT_WARNINGS:YES")

add_library (json_parser STATIC json.c)
