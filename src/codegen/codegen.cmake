set(CODEGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/codegen")
set(GENERATOR_SCRIPT "${CODEGEN_DIR}/generate_files.py")
set(INPUT_CSV_FILE "${CODEGEN_DIR}/procs.csv")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
set(OUTPUT_FILES
    "${OUTPUT_DIR}/real_entrypoints.h"
    "${OUTPUT_DIR}/real_entrypoints.cpp"
)

add_custom_command(
    OUTPUT ${OUTPUT_FILES}
    COMMAND python ARGS "${GENERATOR_SCRIPT}" "${INPUT_CSV_FILE}" "${OUTPUT_DIR}" "${GLBAMBOOZLE_ENABLE_LOGS}" "${GLBAMBOOZLE_ENABLE_LOGS_END}"
    MAIN_DEPENDENCY "${INPUT_CSV_FILE}"
    DEPENDS "${GENERATOR_SCRIPT}"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    COMMENT "Generating ${OUTPUT_DIR}"
)

target_include_directories(glBamboozle PRIVATE "${OUTPUT_DIR}")

set(INPUT_FILES
    ${INPUT_CSV_FILE}
    ${CMAKE_CURRENT_LIST_DIR}/codegen.cmake
    ${CMAKE_CURRENT_LIST_DIR}/generate_files.py
)
target_sources(glBamboozle PRIVATE ${INPUT_FILES} ${OUTPUT_FILES})
source_group(codegen/in FILES ${INPUT_FILES})
source_group(codegen/out FILES ${OUTPUT_FILES})
