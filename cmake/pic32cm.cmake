include(CMakeParseArguments)

option(DEBUG_HARDFAULTS "Enable debugging of hard faults via custom handler")
option(SEMIHOSTING "Enable semihosting")

message("-- DEBUG_HARDFAULTS: ${DEBUG_HARDFAULTS}, SEMIHOSTING: ${SEMIHOSTING}")

set(CMSIS_DIR "${MAIN_LIB_DIR}/main/CMSIS")

set(CMSIS_INCLUDE_DIR2 "${MAIN_SRC_DIR}/../vendor/PIC32CM6408MC00032/include")


set(PIC32CM_STARTUP_DIR "${MAIN_SRC_DIR}/startup")

# XXX: This code is not PIC32CM specific
main_sources(PIC32CM_ASYNCFATFS_SRC
        io/asyncfatfs/asyncfatfs.c
        io/asyncfatfs/fat_standard.c
        )

set(PIC32CM_INCLUDE_DIRS
    "${MAIN_SRC_DIR}/../vendor/PIC32CM6408MC00032/include"
    "${MAIN_SRC_DIR}/../vendor/CMSIS/Core/Include"
    "${MAIN_SRC_DIR}/target"
)

set(PIC32CM_DEFINITIONS
        )
set(PIC32CM_DEFAULT_HSE_MHZ 8)
set(PIC32CM_LINKER_DIR "${MAIN_SRC_DIR}/target/link")
set(PIC32CM_COMPILE_OPTIONS
        -ffunction-sections
        -fdata-sections
        -fno-common
        )

set(PIC32CM_LINK_LIBRARIES
        -lm
        -lc
        )

if(SEMIHOSTING)
    list(APPEND PIC32CM_LINK_LIBRARIES --specs=rdimon.specs -lrdimon)
    list(APPEND PIC32CM_DEFINITIONS SEMIHOSTING)
else()
    list(APPEND PIC32CM_LINK_LIBRARIES -lnosys)
endif()

set(PIC32CM_LINK_OPTIONS
        -nostartfiles
        -specs=nano.specs
        -specs=nosys.specs
        -static
        -Wl,-gc-sections
        -Wl,-L${PIC32CM_LINKER_DIR}
        -Wl,--cref
        -Wl,--no-wchar-size-warning
        -Wl,--print-memory-usage
        )

macro(get_stm32_target_features output_var dir target_name)
    execute_process(COMMAND "${CMAKE_C_COMPILER}" -E -dD -D${ARGV2} "${ARGV1}/target.h"
            ERROR_VARIABLE _errors
            RESULT_VARIABLE _result
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE _contents)

    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "error extracting features for stm32 target ${ARGV2}: ${_errors}")
    endif()

    string(REGEX MATCH "#define[\t ]+USE_VCP" HAS_VCP ${_contents})
    if(HAS_VCP)
        list(APPEND ${ARGV0} VCP)
    endif()
    string(REGEX MATCH "define[\t ]+USE_FLASHFS" HAS_FLASHFS ${_contents})
    if(HAS_FLASHFS)
        list(APPEND ${ARGV0} FLASHFS)
    endif()
    string(REGEX MATCH "define[\t ]+USE_SDCARD" HAS_SDCARD ${_contents})
    if (HAS_SDCARD)
        list(APPEND ${ARGV0} SDCARD)
        string(REGEX MATCH "define[\t ]+USE_SDCARD_SDIO" HAS_SDIO ${_contents})
        if (HAS_SDIO)
            list(APPEND ${ARGV0} SDIO)
        endif()
    endif()
    if(HAS_FLASHFS OR HAS_SDCARD)
        list(APPEND ${ARGV0} MSC)
    endif()
endmacro()

function(get_stm32_flash_size out size)
    # 4: 16, 6: 32, 8: 64, B: 128, C: 256, D: 384, E: 512, F: 768, G: 1024, H: 1536, I: 2048 KiB
    string(TOUPPER ${size} s)
    if(${s} STREQUAL "4")
        set(${out} 16 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "6")
        set(${out} 32 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "8")
        set(${out} 64 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "8")
        set(${out} 64 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "B")
        set(${out} 128 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "C")
        set(${out} 256 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "D")
        set(${out} 384 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "E")
        set(${out} 512 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "F")
        set(${out} 768 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "G")
        set(${out} 1024 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "H")
        set(${out} 1536 PARENT_SCOPE)
        return()
    endif()
    if(${s} STREQUAL "I")
        set(${out} 2048 PARENT_SCOPE)
        return()
    endif()
endfunction()

function(add_hex_target name exe hex)
    add_custom_target(${name} ALL
            cmake -E env PATH="$ENV{PATH}"
            # TODO: Overriding the start address with --set-start 0x08000000
            # seems to be required due to some incorrect assumptions about .hex
            # files in the configurator. Verify wether that's the case and fix
            # the bug in configurator or delete this comment.
            ${CMAKE_OBJCOPY} -Oihex --set-start 0x08000000 $<TARGET_FILE:${exe}> ${hex}
            BYPRODUCTS ${hex}
            )
endfunction()

function(add_bin_target name exe bin)
    add_custom_target(${name}
            cmake -E env PATH="$ENV{PATH}"
            ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${exe}> ${bin}
            BYPRODUCTS ${bin}
            )
endfunction()

function(generate_map_file target)
    if(CMAKE_VERSION VERSION_LESS 3.15)
        set(map "$<TARGET_FILE:${target}>.map")
    else()
        set(map "$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.map")
    endif()
    target_link_options(${target} PRIVATE "-Wl,-Map,${map}")
endfunction()

function(set_linker_script target script)
    set(script_path ${PIC32CM_LINKER_DIR}/${args_LINKER_SCRIPT}.ld)
    if(NOT EXISTS ${script_path})
        message(FATAL_ERROR "linker script ${script_path} doesn't exist")
    endif()
    set_target_properties(${target} PROPERTIES LINK_DEPENDS ${script_path})
    target_link_options(${elf_target} PRIVATE -T${script_path})
endfunction()

function(add_stm32_executable)
    cmake_parse_arguments(
            args
            # Boolean arguments
            ""
            # Single value arguments
            "FILENAME;NAME;OPTIMIZATION;OUTPUT_BIN_FILENAME;OUTPUT_HEX_FILENAME;OUTPUT_TARGET_NAME"
            # Multi-value arguments
            "COMPILE_DEFINITIONS;COMPILE_OPTIONS;INCLUDE_DIRECTORIES;LINK_OPTIONS;LINKER_SCRIPT;SOURCES"
            # Start parsing after the known arguments
            ${ARGN}
    )
    set(elf_target ${args_NAME}.elf)
    add_executable(${elf_target})
    target_sources(${elf_target} PRIVATE ${args_SOURCES})
    target_include_directories(${elf_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${args_INCLUDE_DIRECTORIES} ${PIC32CM_INCLUDE_DIRS})
    target_compile_definitions(${elf_target} PRIVATE ${args_COMPILE_DEFINITIONS})
    target_compile_options(${elf_target} PRIVATE ${PIC32CM_COMPILE_OPTIONS} ${args_COMPILE_OPTIONS})
    if(WARNINGS_AS_ERRORS)
        target_compile_options(${elf_target} PRIVATE -Werror)
    endif()
    if (IS_RELEASE_BUILD)
        target_compile_options(${elf_target} PRIVATE ${args_OPTIMIZATION})
        target_link_options(${elf_target} PRIVATE ${args_OPTIMIZATION})
    endif()
    target_link_libraries(${elf_target} PRIVATE ${PIC32CM_LINK_LIBRARIES})
    target_link_options(${elf_target} PRIVATE ${PIC32CM_LINK_OPTIONS} ${args_LINK_OPTIONS})
    generate_map_file(${elf_target})
    set_linker_script(${elf_target} ${args_LINKER_SCRIPT})
    if(args_FILENAME)
        set(basename ${CMAKE_BINARY_DIR}/${args_FILENAME})
        set(hex_filename ${basename}.hex)
        add_hex_target(${args_NAME} ${elf_target} ${hex_filename})
        set(bin_filename ${basename}.bin)
        add_bin_target(${args_NAME}.bin ${elf_target} ${bin_filename})
    endif()
    if(args_OUTPUT_BIN_FILENAME)
        set(${args_OUTPUT_BIN_FILENAME} ${bin_filename} PARENT_SCOPE)
    endif()
    if(args_OUTPUT_TARGET_NAME)
        set(${args_OUTPUT_TARGET_NAME} ${elf_target} PARENT_SCOPE)
    endif()
    if(args_OUTPUT_HEX_FILENAME)
        set(${args_OUTPUT_HEX_FILENAME} ${hex_filename} PARENT_SCOPE)
    endif()
endfunction()

function(target_mcu)
    if(NOT arm-none-eabi STREQUAL TOOLCHAIN)
        return()
    endif()
    # Parse keyword arguments
    cmake_parse_arguments(
            args
            # Boolean arguments
            "DISABLE_MSC;BOOTLOADER"
            # Single value arguments
            "HSE_MHZ;LINKER_SCRIPT;NAME;OPENOCD_TARGET;OPTIMIZATION;STARTUP;SVD"
            # Multi-value arguments
            "COMPILE_DEFINITIONS;COMPILE_OPTIONS;INCLUDE_DIRECTORIES;LINK_OPTIONS;SOURCES;MSC_SOURCES;MSC_INCLUDE_DIRECTORIES;VCP_SOURCES;VCP_INCLUDE_DIRECTORIES"
            # Start parsing after the known arguments
            ${ARGN}
    )
    set(name ${args_NAME})

    if (args_HSE_MHZ)
        set(hse_mhz ${args_HSE_MHZ})
    else()
        set(hse_mhz ${PIC32CM_DEFAULT_HSE_MHZ})
    endif()

    set(target_sources ${PIC32CM_STARTUP_DIR}/${args_STARTUP})
    list(APPEND target_sources ${args_SOURCES})
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    list(APPEND target_sources ${target_c_sources} ${target_h_sources})

    set(target_include_directories ${args_INCLUDE_DIRECTORIES})

    set(target_definitions ${PIC32CM_DEFINITIONS} ${COMMON_COMPILE_DEFINITIONS})

    get_stm32_target_features(features "${CMAKE_CURRENT_SOURCE_DIR}" ${name})
    set_property(TARGET ${elf_target} PROPERTY FEATURES ${features})

    math(EXPR hse_value "${hse_mhz} * 1000000")
    list(APPEND target_definitions "HSE_VALUE=${hse_value}")
    if(args_COMPILE_DEFINITIONS)
        list(APPEND target_definitions ${args_COMPILE_DEFINITIONS})
    endif()
    if(DEBUG_HARDFAULTS)
        list(APPEND target_definitions DEBUG_HARDFAULTS)
    endif()

    string(TOLOWER ${PROJECT_NAME} lowercase_project_name)
    set(binary_name ${lowercase_project_name}_${FIRMWARE_VERSION}_${name})
    if(DEFINED BUILD_SUFFIX AND NOT "" STREQUAL "${BUILD_SUFFIX}")
        set(binary_name "${binary_name}_${BUILD_SUFFIX}")
    endif()

    # Main firmware
    add_stm32_executable(
            NAME ${name}
            FILENAME ${binary_name}
            SOURCES ${target_sources} ${msc_sources} ${COMMON_SRC}
            COMPILE_DEFINITIONS ${target_definitions}
            COMPILE_OPTIONS ${args_COMPILE_OPTIONS}
            INCLUDE_DIRECTORIES ${target_include_directories}
            LINK_OPTIONS ${args_LINK_OPTIONS}
            LINKER_SCRIPT ${args_LINKER_SCRIPT}
            OPTIMIZATION ${args_OPTIMIZATION}

            OUTPUT_HEX_FILENAME main_hex_filename
            OUTPUT_TARGET_NAME main_target_name
    )

    set_property(TARGET ${main_target_name} PROPERTY OPENOCD_TARGET ${args_OPENOCD_TARGET})
    set_property(TARGET ${main_target_name} PROPERTY OPENOCD_DEFAULT_INTERFACE stlink)
    set_property(TARGET ${main_target_name} PROPERTY SVD ${args_SVD})

    setup_firmware_target(${main_target_name} ${name} ${ARGN})

    if(args_BOOTLOADER)
        # Bootloader for the target
        set(bl_suffix _bl)
        add_stm32_executable(
                NAME ${name}${bl_suffix}
                FILENAME ${binary_name}${bl_suffix}
                SOURCES ${target_sources} ${BOOTLOADER_SOURCES}
                COMPILE_DEFINITIONS ${target_definitions} BOOTLOADER MSP_FIRMWARE_UPDATE
                COMPILE_OPTIONS ${args_COMPILE_OPTIONS}
                INCLUDE_DIRECTORIES ${target_include_directories}
                LINK_OPTIONS ${args_LINK_OPTIONS}
                LINKER_SCRIPT ${args_LINKER_SCRIPT}${bl_suffix}
                OPTIMIZATION ${args_OPTIMIZATION}

                OUTPUT_BIN_FILENAME bl_bin_filename
                OUTPUT_HEX_FILENAME bl_hex_filename
                OUTPUT_TARGET_NAME bl_target_name
        )
        setup_executable(${bl_target_name} ${name})

        # Main firmware, but for running with the bootloader
        set(for_bl_suffix _for_bl)
        add_stm32_executable(
                NAME ${name}${for_bl_suffix}
                FILENAME ${binary_name}${for_bl_suffix}
                SOURCES ${target_sources} ${msc_sources} ${COMMON_SRC}
                COMPILE_DEFINITIONS ${target_definitions} MSP_FIRMWARE_UPDATE
                COMPILE_OPTIONS ${args_COMPILE_OPTIONS}
                INCLUDE_DIRECTORIES ${target_include_directories}
                LINK_OPTIONS ${args_LINK_OPTIONS}
                LINKER_SCRIPT ${args_LINKER_SCRIPT}${for_bl_suffix}
                OPTIMIZATION ${args_OPTIMIZATION}

                OUTPUT_BIN_FILENAME for_bl_bin_filename
                OUTPUT_HEX_FILENAME for_bl_hex_filename
                OUTPUT_TARGET_NAME for_bl_target_name
        )
        setup_executable(${for_bl_target_name} ${name})

        # Combined with bootloader and main firmware
        set(with_bl_suffix _with_bl)
        set(combined_hex ${CMAKE_BINARY_DIR}/${binary_name}${with_bl_suffix}.hex)
        set(with_bl_target ${name}${with_bl_suffix})
        add_custom_target(${with_bl_target}
                ${CMAKE_SOURCE_DIR}/src/utils/combine_tool ${bl_bin_filename} ${for_bl_bin_filename} ${combined_hex}
                BYPRODUCTS ${combined_hex}
                )
        add_dependencies(${with_bl_target} ${bl_target_name} ${for_bl_target_name})
    endif()

    # clean_<target>
    set(generator_cmd "")
    if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        set(generator_cmd "make")
    elseif(CMAKE_GENERATOR STREQUAL "Ninja")
        set(generator_cmd "ninja")
    endif()
    if (NOT generator_cmd STREQUAL "")
        set(clean_target "clean_${name}")
        add_custom_target(${clean_target}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMAND ${generator_cmd} clean
                COMMENT "Removing intermediate files for ${name}")
        set_property(TARGET ${clean_target} PROPERTY
                EXCLUDE_FROM_ALL 1
                EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
endfunction()


include(cortex-m0plus)
#include(stm32-stdperiph)

set(PIC32CM_MC00_CMSIS_DEVICE_DIR "${MAIN_LIB_DIR}/main/PIC32CM_MC00/Drivers/CMSIS/Device/ST/PIC32CM_MC00xx")

main_sources(PIC32CM_MC00_SRC
    ../vendor/PIC32CM6408MC00032/gcc/system_pic32cm6408mc00032.c
    ../vendor/PIC32CM6408MC00032/gcc/system_pic32cm6408mc00032.c

    target/drivers/plib_adc0.c
    target/drivers/plib_port.c
    target/drivers/plib_clock.c
    target/drivers/plib_sercom_usart.c
    target/drivers/plib_tcc0.c
    target/drivers/sercom_spi_master.c
    target/drivers/plib_nvic.c
    target/drivers/plib_dmac.c
    target/drivers/timer.c
    target/device.c
#
#    drivers/adc_stm32f4xx.c
#    drivers/adc_stm32f4xx.c
#    drivers/bus_i2c_stm32f40x.c
#    drivers/serial_uart_stm32f4xx.c
#    drivers/system_stm32f4xx.c
#    drivers/timer.c
#    drivers/timer_impl_stdperiph.c
#    drivers/timer_stm32f4xx.c
#    drivers/uart_inverter.c
#    drivers/dma_stm32f4xx.c
#    drivers/sdcard/sdmmc_sdio_f4xx.c
)

set(PIC32CM_MC00_INCLUDE_DIRS
    "drivers"
    "${CMSIS_INCLUDE_DIR}"
)

set(PIC32CM_MC00_DEFINITIONS
    ${CORTEX_M0_DEFINITIONS}
    PIC32CM_MC00
)

function(target_pic32cm)
    target_mcu(
        SOURCES ${PIC32CM_STDPERIPH_SRC} ${PIC32CM_MC00_SRC}
        COMPILE_DEFINITIONS ${PIC32CM_MC00_DEFINITIONS}
        COMPILE_OPTIONS ${CORTEX_M0_COMMON_OPTIONS} ${CORTEX_M0_COMPILE_OPTIONS}
        INCLUDE_DIRECTORIES ${PIC32CM_MC00_INCLUDE_DIRS}
        LINK_OPTIONS ${CORTEX_M0_COMMON_OPTIONS} ${CORTEX_M0_LINK_OPTIONS}

        OPTIMIZATION -Og

        OPENOCD_TARGET pic32cm

        ${ARGN}
    )
endfunction()

set(PIC32CM_MC0011_OR_F427_STDPERIPH_SRC ${PIC32CM_MC00_STDPERIPH_SRC})
set(PIC32CM_MC0011_OR_F427_STDPERIPH_SRC_EXCLUDES "stm32f4xx_fsmc.c")
exclude_basenames(PIC32CM_MC0011_OR_F427_STDPERIPH_SRC ${PIC32CM_MC0011_OR_F427_STDPERIPH_SRC_EXCLUDES})

set(PIC32CM_MC0027_COMPILE_DEFINITIONS
    PIC32CM_MC0027_437xx
    MCU_FLASH_SIZE=1024
)
function(target_pic32cm_mc00 name)
    target_pic32cm(
        NAME ${name}
        STARTUP startup_pic32cm6408mc00032.c
        SOURCES ${PIC32CM_MC0011_OR_F427_STDPERIPH_SRC}
        COMPILE_DEFINITIONS ${PIC32CM_MC0027_COMPILE_DEFINITIONS}
        LINKER_SCRIPT pic32cm6408mc00032_flash
        SVD PIC32CM_MC0011
        ${ARGN}
    )
endfunction()
