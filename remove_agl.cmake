# Script to remove AGL framework from link commands on macOS 15+
# The AGL framework was deprecated and removed, causing link errors

set(LINK_FILE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir/link.txt")

if(EXISTS "${LINK_FILE}")
    file(READ "${LINK_FILE}" LINK_COMMAND)

    # Remove all references to AGL framework
    string(REPLACE "-framework AGL" "" LINK_COMMAND "${LINK_COMMAND}")
    string(REPLACE "-framework  AGL" "" LINK_COMMAND "${LINK_COMMAND}")
    string(REPLACE "AGL.framework" "" LINK_COMMAND "${LINK_COMMAND}")

    # Write back the cleaned link command
    file(WRITE "${LINK_FILE}" "${LINK_COMMAND}")

    message(STATUS "Removed AGL framework references from ${TARGET_NAME} link command")
endif()
