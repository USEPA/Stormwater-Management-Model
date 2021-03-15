


#cmake_minimum_required(VERSION 3.19)
#project(openmp)

include(FetchContent)


FetchContent_Declare(
	openmp
    URL https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/openmp-11.1.0.src.tar.xz
    URL_HASH SHA256=d187483b75b39acb3ff8ea1b7d98524d95322e3cb148842957e9b0fbb866052e
)

set(OPENMP_STANDALONE_BUILD TRUE)
set(LIBOMP_INSTALL_ALIASES OFF)
FetchContent_MakeAvailable(openmp)

