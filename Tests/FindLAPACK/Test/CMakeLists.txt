cmake_minimum_required(VERSION 3.10)
project(TestFindLAPACK C)
include(CTest)

find_package(LAPACK REQUIRED)

# Configure the test case to match LAPACK's integer type ABI.
# FindLAPACK defaults to 32-bit integers if BLA_SIZEOF_INTEGER
# is not already set in the cache.
if(NOT BLA_SIZEOF_INTEGER)
  set(BLA_SIZEOF_INTEGER 4)
endif()

add_executable(test_tgt main.c)
target_link_libraries(test_tgt LAPACK::LAPACK)
target_compile_definitions(test_tgt PUBLIC BLA_SIZEOF_INTEGER=${BLA_SIZEOF_INTEGER})
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_link_libraries(test_var PRIVATE ${LAPACK_LIBRARIES})
target_compile_definitions(test_var PUBLIC BLA_SIZEOF_INTEGER=${BLA_SIZEOF_INTEGER})
add_test(NAME test_var COMMAND test_var)

if((BLA_VENDOR STREQUAL "Intel10_64lp") OR
   (BLA_VENDOR STREQUAL "All" AND EXPECT_All STREQUAL "Intel10_64lp"))
  if(NOT LAPACK_LIBRARIES MATCHES "^(-Wl,--start-group;)?[^;]*mkl_intel_lp64")
    message(FATAL_ERROR "LAPACK_LIBRARIES does not start in mkl_intel_lp64:\n ${LAPACK_LIBRARIES}")
  endif()
elseif((BLA_VENDOR STREQUAL "Intel10_64ilp") OR
   (BLA_VENDOR STREQUAL "All" AND EXPECT_All STREQUAL "Intel10_64ilp"))
  if(NOT LAPACK_LIBRARIES MATCHES "^(-Wl,--start-group;)?[^;]*mkl_intel_ilp64")
    message(FATAL_ERROR "LAPACK_LIBRARIES does not start in mkl_intel_ilp64:\n ${LAPACK_LIBRARIES}")
  endif()
endif()
