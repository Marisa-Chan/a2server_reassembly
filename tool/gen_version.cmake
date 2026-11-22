# Generates build_version.cpp with g_BuildVersion = "<commit>" or "<commit>+<tree>".
# Invoked at configure time and on every build via the gen_build_version target.

execute_process(
  COMMAND git rev-parse --short=8 HEAD
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE head_res
  OUTPUT_VARIABLE git_head
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT head_res EQUAL 0 OR git_head STREQUAL "")
  set(build_version "unknown")
else()
  set(build_version "${git_head}")

  execute_process(
    COMMAND git stash create
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE stash_res
    OUTPUT_VARIABLE stash_sha
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  if(stash_res EQUAL 0 AND NOT stash_sha STREQUAL "")
    execute_process(
      COMMAND git rev-parse --short=6 "${stash_sha}^{tree}"
      WORKING_DIRECTORY "${SOURCE_DIR}"
      RESULT_VARIABLE tree_res
      OUTPUT_VARIABLE tree_sha
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(tree_res EQUAL 0 AND NOT tree_sha STREQUAL "")
      set(build_version "${git_head}+${tree_sha}")
    else()
      set(build_version "${git_head}+dirty")
    endif()
  endif()
endif()

set(new_content "#include \"patch/build_version.h\"\n\nconst char* const g_BuildVersion = \"${build_version}\";\n")

get_filename_component(out_dir "${OUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${out_dir}")

set(old_content "")
if(EXISTS "${OUT_FILE}")
  file(READ "${OUT_FILE}" old_content)
endif()

if(NOT old_content STREQUAL new_content)
  file(WRITE "${OUT_FILE}" "${new_content}")
endif()
