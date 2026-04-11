add_library(arch STATIC)

file(GLOB ARCH_POSIX "posix/*.cpp")

target_sources(arch PRIVATE
        ${ARCH_POSIX}
)

target_include_directories(arch PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(arch PUBLIC
        kernel
)