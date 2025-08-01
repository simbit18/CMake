include(RunCMake)

set(RunCMake_GENERATOR "Ninja")
set(RunCMake_GENERATOR_IS_MULTI_CONFIG 0)

# Detect ninja version so we know what tests can be supported.
execute_process(
  COMMAND "${RunCMake_MAKE_PROGRAM}" --version
  OUTPUT_VARIABLE ninja_out
  ERROR_VARIABLE ninja_out
  RESULT_VARIABLE ninja_res
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
if(ninja_res EQUAL 0 AND "x${ninja_out}" MATCHES "^x[0-9]+\\.[0-9]+")
  set(ninja_version "${ninja_out}")
  message(STATUS "ninja version: ${ninja_version}")
else()
  message(FATAL_ERROR "'ninja --version' reported:\n${ninja_out}")
endif()

# Sanitize NINJA_STATUS since we expect default behavior.
unset(ENV{NINJA_STATUS})

if(CMAKE_HOST_WIN32)
  run_cmake(SelectCompilerWindows)
else()
  run_cmake(SelectCompilerUNIX)
endif()

function(run_NinjaToolMissing)
  set(RunCMake_MAKE_PROGRAM ninja-tool-missing)
  run_cmake(NinjaToolMissing)
endfunction()
run_NinjaToolMissing()

function(run_Intl)
  run_cmake(Intl)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Intl-build)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(Intl-build ${CMAKE_COMMAND} --build .)
endfunction()
run_Intl()

if(WIN32)
  if(RunCMake_MAKE_PROGRAM)
    set(maybe_MAKE_PROGRAM "-DRunCMake_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}")
  endif()
  run_cmake_script(ShowIncludes-437-ClangCl-17 -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-437-ClangCl-18 -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-437-English -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-437-French -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-437-German -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-437-Italian -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-437-WineMSVC -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-54936-Chinese -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-65001-Chinese -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-65001-French -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  run_cmake_script(ShowIncludes-65001-Japanese -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  if(NOT CMake_TEST_NO_CODEPAGE_9xx)
    run_cmake_script(ShowIncludes-932-Japanese -DshowIncludes=${showIncludes} ${maybe_MAKE_PROGRAM})
  endif()
  unset(maybe_MAKE_PROGRAM)
endif()

function(run_NoWorkToDo)
  run_cmake(NoWorkToDo)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/NoWorkToDo-build)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(NoWorkToDo-build ${CMAKE_COMMAND} --build .)
  run_cmake_command(NoWorkToDo-nowork ${CMAKE_COMMAND} --build . -- -d explain)
endfunction()
run_NoWorkToDo()

function(run_VerboseBuild)
  run_cmake(VerboseBuild)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VerboseBuild-build)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(VerboseBuild-build ${CMAKE_COMMAND} --build . -v --clean-first)
  run_cmake_command(VerboseBuild-nowork ${CMAKE_COMMAND} --build . --verbose)
endfunction()
run_VerboseBuild()

function(run_VerboseBuildShort)
  run_cmake(VerboseBuildShort)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VerboseBuildShort-build)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(VerboseBuildShort-build ${CMAKE_COMMAND} --build . -v --clean-first)
  run_cmake_command(VerboseBuildShort-nowork ${CMAKE_COMMAND} --build . --verbose)
endfunction()
run_VerboseBuildShort()

function(run_CMP0058 case)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0058-${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(CMP0058-${case})
  run_cmake_command(CMP0058-${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

run_CMP0058(NEW-no)
run_CMP0058(NEW-by)

run_cmake_with_options(CustomCommandDepfile -DCMAKE_BUILD_TYPE=Debug)
run_cmake_with_options(CustomCommandDepfileAsOutput -DCMAKE_BUILD_TYPE=Debug)
run_cmake_with_options(CustomCommandDepfileAsByproduct -DCMAKE_BUILD_TYPE=Debug)
run_cmake(CustomCommandJobPool)
run_cmake(SourceFileJobPool)
run_cmake(JobPoolUsesTerminal)

run_cmake(RspFileC)
run_cmake(RspFileCXX)
if(CMake_TEST_Fortran
    # FIXME(lfortran): The compiler does not support response files.
    AND NOT CMAKE_Fortran_COMPILER_ID STREQUAL "LFortran"
    )
  run_cmake(RspFileFortran)
endif()

function(run_CommandConcat)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CommandConcat-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(CommandConcat)
  run_cmake_command(CommandConcat-build ${CMAKE_COMMAND} --build .)
endfunction()
run_CommandConcat()

function(run_SubDir)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SubDir-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(SubDir)
  if(WIN32)
    set(SubDir_all [[SubDir\all]])
    set(SubDir_test [[SubDir\test]])
    set(SubDir_install [[SubDir\install]])
    set(SubDirBinary_test [[SubDirBinary\test]])
    set(SubDirBinary_all [[SubDirBinary\all]])
    set(SubDirBinary_install [[SubDirBinary\install]])
  else()
    set(SubDir_all [[SubDir/all]])
    set(SubDir_test [[SubDir/test]])
    set(SubDir_install [[SubDir/install]])
    set(SubDirBinary_all [[SubDirBinary/all]])
    set(SubDirBinary_test [[SubDirBinary/test]])
    set(SubDirBinary_install [[SubDirBinary/install]])
  endif()
  run_cmake_command(SubDir-build ${CMAKE_COMMAND} --build . --target ${SubDir_all})
  run_cmake_command(SubDir-test ${CMAKE_COMMAND} --build . --target ${SubDir_test})
  run_cmake_command(SubDir-install ${CMAKE_COMMAND} --build . --target ${SubDir_install})
  run_cmake_command(SubDirBinary-build ${CMAKE_COMMAND} --build . --target ${SubDirBinary_all})
  run_cmake_command(SubDirBinary-test ${CMAKE_COMMAND} --build . --target ${SubDirBinary_test})
  run_cmake_command(SubDirBinary-install ${CMAKE_COMMAND} --build . --target ${SubDirBinary_install})
endfunction()
run_SubDir()

function(run_ninja dir)
  execute_process(
    COMMAND "${RunCMake_MAKE_PROGRAM}" ${ARGN}
    WORKING_DIRECTORY "${dir}"
    OUTPUT_VARIABLE ninja_stdout
    ERROR_VARIABLE ninja_stderr
    RESULT_VARIABLE ninja_result
    )
  if(NOT ninja_result EQUAL 0)
    message(STATUS "
============ beginning of ninja's stdout ============
${ninja_stdout}
=============== end of ninja's stdout ===============
")
    message(STATUS "
============ beginning of ninja's stderr ============
${ninja_stderr}
=============== end of ninja's stderr ===============
")
    message(FATAL_ERROR
      "top ninja build failed exited with status ${ninja_result}")
  endif()
  set(ninja_stdout "${ninja_stdout}" PARENT_SCOPE)
endfunction()

function (run_LooseObjectDepends)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LooseObjectDepends-build)
  run_cmake(LooseObjectDepends)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" "CMakeFiles/top.dir/top.c${CMAKE_C_OUTPUT_EXTENSION}")
  if (EXISTS "${RunCMake_TEST_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}dep${CMAKE_SHARED_LIBRARY_SUFFIX}")
    message(FATAL_ERROR
      "The `dep` library was created when requesting an object file to be "
      "built; this should no longer be necessary.")
  endif ()
  if (EXISTS "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/dep.dir/dep.c${CMAKE_C_OUTPUT_EXTENSION}")
    message(FATAL_ERROR
      "The `dep.c` object file was created when requesting an object file to "
      "be built; this should no longer be necessary.")
  endif ()
endfunction ()
run_LooseObjectDepends()

function (run_LooseObjectDependsShort)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LooseObjectDependsShort-build)
  run_cmake(LooseObjectDependsShort)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" ".o/e86fd702/b1363cd8${CMAKE_C_OUTPUT_EXTENSION}")
  if (EXISTS "${RunCMake_TEST_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}dep${CMAKE_SHARED_LIBRARY_SUFFIX}")
    message(FATAL_ERROR
      "The `dep` library was created when requesting an object file to be "
      "built; this should no longer be necessary.")
  endif ()
  if (EXISTS "${RunCMake_TEST_BINARY_DIR}/.o/600bd702/d56215${CMAKE_C_OUTPUT_EXTENSION}")
    message(FATAL_ERROR
      "The `dep.c` object file was created when requesting an object file to "
      "be built; this should no longer be necessary.")
  endif ()
endfunction ()
run_LooseObjectDependsShort()

function (run_CustomCommandExplictDepends)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CustomCommandExplicitDepends-build)
  run_cmake(CustomCommandExplicitDepends)

  set(DEP_LIB "${RunCMake_TEST_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}dep${CMAKE_SHARED_LIBRARY_SUFFIX}")

  run_ninja("${RunCMake_TEST_BINARY_DIR}" "command-option.h")
  if (EXISTS "${DEP_LIB}")
    message(FATAL_ERROR
      "The `dep` library was created when requesting a custom command to be "
      "generated; this should no longer be necessary when passing "
      "DEPENDS_EXPLICIT_ONLY option.")
  endif ()

  run_ninja("${RunCMake_TEST_BINARY_DIR}" "command-variable-on.h")
  if (EXISTS "${DEP_LIB}")
    message(FATAL_ERROR
      "The `dep` library was created when requesting a custom command to be "
      "generated; this should no longer be necessary when setting "
      "CMAKE_ADD_CUSTOM_COMMAND_DEPENDS_EXPLICIT_ONLY variable to ON.")
  endif ()

  run_ninja("${RunCMake_TEST_BINARY_DIR}" "command-variable-off.h")
  if (NOT EXISTS "${DEP_LIB}")
    message(FATAL_ERROR
      "The `dep` library was not created when requesting a custom command to be "
      "generated; this should be necessary when setting "
      "CMAKE_ADD_CUSTOM_COMMAND_DEPENDS_EXPLICIT_ONLY variable to OFF.")
  endif ()
endfunction ()
run_CustomCommandExplictDepends()

function (run_AssumedSources)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/AssumedSources-build)
  run_cmake(AssumedSources)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" "${RunCMake_TEST_BINARY_DIR}/target.c")
  if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/target.c")
    message(FATAL_ERROR
      "Dependencies for an assumed source did not hook up properly for 'target.c'.")
  endif ()
  run_ninja("${RunCMake_TEST_BINARY_DIR}" "${RunCMake_TEST_BINARY_DIR}/target-no-depends.c")
  if (EXISTS "${RunCMake_TEST_BINARY_DIR}/target-no-depends.c")
    message(FATAL_ERROR
      "Dependencies for an assumed source were magically hooked up for 'target-no-depends.c'.")
  endif ()
endfunction ()
run_AssumedSources()

function(sleep delay)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E sleep ${delay}
    RESULT_VARIABLE result
    )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "failed to sleep for ${delay} second.")
  endif()
endfunction(sleep)

macro(ninja_escape_path path out)
  string(REPLACE "\$ " "\$\$" "${out}" "${path}")
  string(REPLACE " " "\$ " "${out}" "${${out}}")
  string(REPLACE ":" "\$:" "${out}" "${${out}}")
endmacro()

macro(shell_escape string out)
  string(REPLACE "\"" "\\\"" "${out}" "${string}")
endmacro()

function(run_sub_cmake test ninja_output_path_prefix)
  set(top_build_dir "${RunCMake_BINARY_DIR}/${test}-build/")
  file(REMOVE_RECURSE "${top_build_dir}")
  file(MAKE_DIRECTORY "${top_build_dir}")

  ninja_escape_path("${ninja_output_path_prefix}"
    escaped_ninja_output_path_prefix)

  # Generate top build ninja file.
  set(top_build_ninja "${top_build_dir}/build.ninja")
  shell_escape("${top_build_ninja}" escaped_top_build_ninja)
  set(build_ninja_dep "${top_build_dir}/build_ninja_dep")
  ninja_escape_path("${build_ninja_dep}" escaped_build_ninja_dep)
  shell_escape("${CMAKE_COMMAND}" escaped_CMAKE_COMMAND)
  file(WRITE "${build_ninja_dep}" "fake dependency of top build.ninja file\n")
  if(WIN32)
    set(cmd_prefix "cmd.exe /C \"")
    set(cmd_suffix "\"")
  else()
    set(cmd_prefix "")
    set(cmd_suffix "")
  endif()
  set(fs_delay 3) # We assume the system as 1 sec timestamp resolution.
  file(WRITE "${top_build_ninja}" "\
subninja ${escaped_ninja_output_path_prefix}/build.ninja
default ${escaped_ninja_output_path_prefix}/all

# Sleep for long enough before regenerating to make sure the timestamp of
# the top build.ninja will be strictly greater than the timestamp of the
# sub/build.ninja file.
rule RERUN
  command = ${cmd_prefix}\"${escaped_CMAKE_COMMAND}\" -E sleep ${fs_delay} && \"${escaped_CMAKE_COMMAND}\" -E touch \"${escaped_top_build_ninja}\"${cmd_suffix}
  description = Testing regeneration
  generator = 1

build build.ninja: RERUN ${escaped_build_ninja_dep} || ${escaped_ninja_output_path_prefix}/build.ninja
  pool = console
")

  # Run sub cmake project.
  set(RunCMake_TEST_OPTIONS "-DCMAKE_NINJA_OUTPUT_PATH_PREFIX=${ninja_output_path_prefix}")
  set(RunCMake_TEST_BINARY_DIR "${top_build_dir}/${ninja_output_path_prefix}")
  run_cmake(${test})

  # Check there is no 'default' statement in Ninja file generated by CMake.
  set(sub_build_ninja "${RunCMake_TEST_BINARY_DIR}/build.ninja")
  file(READ "${sub_build_ninja}" sub_build_ninja_file)
  if(sub_build_ninja_file MATCHES "\ndefault [^\n][^\n]*all\n")
    message(FATAL_ERROR
      "unexpected 'default' statement found in '${sub_build_ninja}'")
  endif()

  # Run ninja from the top build directory.
  run_ninja("${top_build_dir}")

  # Test regeneration rules run in order.
  set(main_cmakelists "${RunCMake_SOURCE_DIR}/CMakeLists.txt")
  sleep(${fs_delay})
  file(TOUCH "${main_cmakelists}")
  file(TOUCH "${build_ninja_dep}")
  run_ninja("${top_build_dir}")
  file(TIMESTAMP "${main_cmakelists}" mtime_main_cmakelists UTC)
  file(TIMESTAMP "${sub_build_ninja}" mtime_sub_build_ninja UTC)
  file(TIMESTAMP "${top_build_ninja}" mtime_top_build_ninja UTC)

  # Check sub build.ninja is regenerated.
  if(mtime_main_cmakelists STRGREATER mtime_sub_build_ninja)
    message(FATAL_ERROR
      "sub build.ninja not regenerated:
  CMakeLists.txt  = ${mtime_main_cmakelists}
  sub/build.ninja = ${mtime_sub_build_ninja}")
  endif()

  # Check top build.ninja is regenerated after sub build.ninja.
  if(NOT mtime_top_build_ninja STRGREATER mtime_sub_build_ninja)
    message(FATAL_ERROR
      "top build.ninja not regenerated strictly after sub build.ninja:
  sub/build.ninja = ${mtime_sub_build_ninja}
  build.ninja     = ${mtime_top_build_ninja}")
  endif()

endfunction()

if("${ninja_version}" VERSION_LESS 1.6)
  message(WARNING "Ninja is too old; skipping rest of test.")
  return()
endif()
if("${ninja_version}" VERSION_LESS 1.12)
  set(maybe_w_dupbuild_err -w dupbuild=err)
endif()

set(ninja_output_path_prefixes "sub")
if(NOT CMAKE_C_COMPILER_ID STREQUAL "OrangeC")
  list(APPEND ninja_output_path_prefixes "sub space")
endif()
foreach(ninja_output_path_prefix IN LISTS ninja_output_path_prefixes)
  run_sub_cmake(Executable "${ninja_output_path_prefix}")
  run_sub_cmake(StaticLib  "${ninja_output_path_prefix}")
  run_sub_cmake(SharedLib "${ninja_output_path_prefix}")
  run_sub_cmake(TwoLibs "${ninja_output_path_prefix}")
  run_sub_cmake(SubDirPrefix "${ninja_output_path_prefix}")
  run_sub_cmake(CustomCommandWorkingDirectory "${ninja_output_path_prefix}")
endforeach(ninja_output_path_prefix)

function (run_PreventTargetAliasesDupBuildRule)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/PreventTargetAliasesDupBuildRule-build)
  run_cmake(PreventTargetAliasesDupBuildRule)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" ${maybe_w_dupbuild_err})
endfunction ()
run_PreventTargetAliasesDupBuildRule()

function (run_PreventConfigureFileDupBuildRule)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/PreventConfigureFileDupBuildRule-build)
  run_cmake(PreventConfigureFileDupBuildRule)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" ${maybe_w_dupbuild_err})
endfunction()
run_PreventConfigureFileDupBuildRule()

function (run_ChangeBuildType)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ChangeBuildType-build)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_BUILD_TYPE:STRING=Debug")
  run_cmake(ChangeBuildType)
  unset(RunCMake_TEST_OPTIONS)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" ${maybe_w_dupbuild_err})
endfunction()
run_ChangeBuildType()

function (run_CustomCommandTargetComments)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CustomCommandTargetComments-build)
  run_cmake(CustomCommandTargetComments)
  unset(RunCMake_TEST_OPTIONS)
  run_ninja("${RunCMake_TEST_BINARY_DIR}" ${maybe_w_dupbuild_err})
  if (NOT ninja_stdout MATCHES [[pre-build: genex; pre-link: genex; Linking C executable hello(\.exe)?; post-build: genex]])
    string(REPLACE "\n" "\n  " ninja_stdout "${ninja_stdout}")
    message(SEND_ERROR
      "Custom command comments are not part of the description:\n"
      "  ${ninja_stdout}"
    )
  endif ()
endfunction()
run_CustomCommandTargetComments()

function(run_QtAutoMocSkipPch)
  set(QtX Qt${CMake_TEST_Qt_version})
  if(CMake_TEST_${QtX}Core_Version VERSION_GREATER_EQUAL 5.15.0)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/QtAutoMocSkipPch-build)
    run_cmake_with_options(QtAutoMocSkipPch
      "-Dwith_qt_version=${CMake_TEST_Qt_version}"
      "-D${QtX}_DIR=${${QtX}_DIR}"
      "-D${QtX}Core_DIR=${${QtX}Core_DIR}"
      "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
    )
    # Build the project.
    run_ninja("${RunCMake_TEST_BINARY_DIR}")
  endif()
endfunction()

if(CMake_TEST_Qt_version)
  run_QtAutoMocSkipPch()
endif()

run_cmake(LINK_OPTIONSWithNewlines)

run_cmake(StaticLibShort)
