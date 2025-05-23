# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckVariableExists
-------------------

Check if the variable exists.

.. command:: check_variable_exists

  .. code-block:: cmake

    check_variable_exists(<var> <variable>)

  Check if the variable ``<var>`` exists and store the result in an internal
  cache variable ``<variable>``.

  This macro is only for ``C`` variables.

The following variables may be set before calling this macro to modify
the way the check is run:

  .. include:: /module/include/CMAKE_REQUIRED_FLAGS.rst

  .. include:: /module/include/CMAKE_REQUIRED_DEFINITIONS.rst

  .. include:: /module/include/CMAKE_REQUIRED_LINK_OPTIONS.rst

  .. include:: /module/include/CMAKE_REQUIRED_LIBRARIES.rst

  .. include:: /module/include/CMAKE_REQUIRED_QUIET.rst

#]=======================================================================]

include_guard(GLOBAL)

macro(CHECK_VARIABLE_EXISTS VAR VARIABLE)
  if(NOT DEFINED "${VARIABLE}")
    set(MACRO_CHECK_VARIABLE_DEFINITIONS
      "-DCHECK_VARIABLE_EXISTS=${VAR} ${CMAKE_REQUIRED_FLAGS}")
    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Looking for ${VAR}")
    endif()
    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_VARIABLE_EXISTS_ADD_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_VARIABLE_EXISTS_ADD_LINK_OPTIONS)
    endif()
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES)
    endif()

    if(CMAKE_REQUIRED_LINK_DIRECTORIES)
      set(_CVE_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(_CVE_LINK_DIRECTORIES)
    endif()

    try_compile(${VARIABLE}
      SOURCES ${CMAKE_ROOT}/Modules/CheckVariableExists.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_VARIABLE_EXISTS_ADD_LINK_OPTIONS}
      ${CHECK_VARIABLE_EXISTS_ADD_LIBRARIES}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_VARIABLE_DEFINITIONS}
      "${_CVE_LINK_DIRECTORIES}"
      )
    unset(_CVE_LINK_DIRECTORIES)
    if(${VARIABLE})
      set(${VARIABLE} 1 CACHE INTERNAL "Have variable ${VAR}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "found")
      endif()
    else()
      set(${VARIABLE} "" CACHE INTERNAL "Have variable ${VAR}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "not found")
      endif()
    endif()
  endif()
endmacro()
