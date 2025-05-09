# strip_string.cmake
file(READ "${INPUT}" contents)
string(REPLACE "${TARGET}" "" contents "${contents}")
file(WRITE "${OUTPUT}" "${contents}")
