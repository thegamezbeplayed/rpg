# Glob all resource files (e.g., PNG, WAV, etc.)
file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.png
    ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ogg
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ttf
    # Add more patterns as needed
)

# List of all copy commands
set(COPY_COMMANDS)

foreach(RESOURCE_FILE ${RESOURCE_FILES})
    # Get path relative to the resources folder
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${RESOURCE_FILE}")
    if(ANDROID)
      set(RESOURCE_BASE "${CMAKE_SOURCE_DIR}/app/src/main/assets")
    else()
      set(RESOURCE_BASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
    # Destination path (preserves relative folder structure)
set(DEST_FILE "${RESOURCE_BASE}/${REL_PATH}")
    # Get destination directory
    get_filename_component(DEST_DIR "${DEST_FILE}" DIRECTORY)

    # Append make_directory and copy commands
    list(APPEND COPY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${RESOURCE_FILE}" "${DEST_FILE}"
    )
endforeach()

# Create a custom target to copy resources on build
add_custom_target(copy_resources ALL
    ${COPY_COMMANDS}
    COMMENT "Copying resource files to runtime output directory"
)

# Optionally make your game depend on it
add_dependencies(${PROJECT_NAME} copy_resources)
