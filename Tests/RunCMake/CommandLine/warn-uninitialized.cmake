set(FOO "${NEW_WARN_FROM_NORMAL_CMAKE_FILE_INSIDE_BRACES}")
string(CONFIGURE "\${NEW_WARN_FROM_STRING_CONFIGURE_INSIDE_BRACES}" OUT3)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/file2.in "\@NEW_WARN_FROM_CONFIGURE_FILE_INSIDE_AT\@")
configure_file(${CMAKE_CURRENT_BINARY_DIR}/file2.in file2.out)
string(CONFIGURE "@NEW_WARN_FROM_STRING_CONFIGURE_INSIDE_AT@" OUT4)
