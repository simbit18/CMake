set(CMAKE_CUDA_LINKER_LAUNCHER "${CMAKE_COMMAND};-E;env;USED_LAUNCHER=1")
include(CUDA-common.cmake)
