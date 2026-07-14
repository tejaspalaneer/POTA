cmake_minimum_required(VERSION 3.15)
project(POTASetup)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fcoroutines)

# --- THE GOLDEN PATHS ---
set(libOTe_DIR "/home/paneer/Desktop/POTA/libOTe/out/build/linux")
set(coproto_DIR "/home/paneer/Desktop/POTA/libOTe/out/build/linux/coproto/coproto")
set(macoro_DIR "/home/paneer/Desktop/POTA/libOTe/out/build/linux/macoro/macoro")
set(function2_DIR "/home/paneer/Desktop/POTA/libOTe/out/install/linux/lib/cmake/function2")
set(Boost_DIR "/home/paneer/Desktop/POTA/libOTe/out/install/linux/lib/cmake/Boost-1.90.0")
set(LIBDIVIDE_INCLUDE_DIRS "/home/paneer/Desktop/POTA/libOTe/out/install/linux/include")
set(SEAL_DIR "/home/paneer/Desktop/POTA/libOTe/out/install/linux/lib/cmake/SEAL-4.1")

set(libsodium_DIR "/home/paneer/Desktop/POTA/libOTe/out/install/linux/lib/cmake/libsodium")
list(APPEND CMAKE_PREFIX_PATH "/home/paneer/Desktop/POTA/libOTe/out/install/linux")

find_package(libOTe REQUIRED)

add_executable(pota_networked pota_networked.cpp)
target_link_libraries(pota_networked PRIVATE 
    "/home/paneer/Desktop/POTA/libOTe/out/install/linux/lib/libsodium.a"
    oc::libOTe
)
target_link_options(pota_networked PRIVATE "-Wl,--exclude-libs,ALL")
