add_library(kernel STATIC )

file(GLOB KERNEL_CORE "core/*.cpp")
file(GLOB KERNEL_MEM "mem/*.cpp")
file(GLOB KERNEL_SYNC "sync/*.cpp")
file(GLOB KERNEL_TIMER "timer/*.cpp")

target_sources(kernel PRIVATE
        ${KERNEL_CORE}
        ${KERNEL_MEM}
        ${KERNEL_SYNC}
        ${KERNEL_TIMER}
)

target_include_directories(kernel PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(kernel PRIVATE
        -Wall
        -Wextra
        -fno-exceptions
        -fno-rtti
)

