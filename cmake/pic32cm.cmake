include(CMakeParseArguments)

option(SEMIHOSTING "Enable semihosting")

message("-- SEMIHOSTING: ${SEMIHOSTING}")

set(PIC32CM_STARTUP_DIR "${MAIN_SRC_DIR}/startup")

set(PIC32CM_INCLUDE_DIRS
    "${MAIN_SRC_DIR}/../vendor/PIC32CM6408MC00032/include"
    "${MAIN_SRC_DIR}/../vendor/CMSIS/Core/Include"
    "${MAIN_SRC_DIR}/target"
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

function(add_arm_executable)
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
    endif()
    if(args_OUTPUT_TARGET_NAME)
        set(${args_OUTPUT_TARGET_NAME} ${elf_target} PARENT_SCOPE)
    endif()
    if(args_OUTPUT_HEX_FILENAME)
        set(${args_OUTPUT_HEX_FILENAME} ${hex_filename} PARENT_SCOPE)
    endif()
endfunction()

include(cortex-m0plus)

main_sources(PIC32CM_MC00_SRC
    target/drivers/plib_adc0.c
    target/drivers/plib_port.c
    target/drivers/plib_clock.c
    target/drivers/plib_sercom_usart.c
    target/drivers/plib_tcc0.c
    target/drivers/sercom_spi_master.c
    target/drivers/plib_nvic.c
    target/drivers/plib_dmac.c
    target/drivers/plib_systick.c
    target/drivers/timer.c
    target/drivers/l9958.c
    target/drivers/dshot.c
    target/drivers/plib_wdt.c
    target/drivers/plib_nvmctrl.c
    target/device.c
)

set(PIC32CM_MC00_INCLUDE_DIRS
    "drivers"
)

set(PIC32CM_MC00_DEFINITIONS
    ${CORTEX_M0_DEFINITIONS}
    PIC32CM_MC00
)

set(PIC32CM_MC0027_COMPILE_DEFINITIONS
    MCU_FLASH_SIZE=64
)

function(target_mcu)
    if(NOT arm-none-eabi STREQUAL TOOLCHAIN)
        return()
    endif()
    # Parse keyword arguments
    cmake_parse_arguments(
            args
            # Boolean arguments
            "BOOTLOADER"
            # Single value arguments
            "LINKER_SCRIPT;NAME;OPENOCD_TARGET;OPTIMIZATION;STARTUP;SVD"
            # Multi-value arguments
            "COMPILE_DEFINITIONS;COMPILE_OPTIONS;INCLUDE_DIRECTORIES;LINK_OPTIONS;SOURCES"
            # Start parsing after the known arguments
            ${ARGN}
    )
    set(name ${args_NAME})

    set(target_sources ${PIC32CM_STARTUP_DIR}/startup_pic32cm6408mc00032.c)
    list(APPEND target_sources ${args_SOURCES})
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    list(APPEND target_sources ${target_c_sources} ${target_h_sources})

    set(target_include_directories ${args_INCLUDE_DIRECTORIES})

    set(target_definitions ${COMMON_COMPILE_DEFINITIONS})

    if(args_COMPILE_DEFINITIONS)
        list(APPEND target_definitions ${args_COMPILE_DEFINITIONS})
    endif()

    string(TOLOWER ${PROJECT_NAME} lowercase_project_name)
    set(binary_name ${lowercase_project_name}_${FIRMWARE_VERSION}_${name})
    if(DEFINED BUILD_SUFFIX AND NOT "" STREQUAL "${BUILD_SUFFIX}")
        set(binary_name "${binary_name}_${BUILD_SUFFIX}")
    endif()

    # Main firmware
    add_arm_executable(
            NAME ${name}
            FILENAME ${binary_name}
            SOURCES ${target_sources} ${COMMON_SRC}
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

function(target_pic32cm_mc00 name)
    target_mcu(
            NAME ${name}
            SOURCES ${PIC32CM_STDPERIPH_SRC} ${PIC32CM_MC00_SRC}
            COMPILE_DEFINITIONS ${PIC32CM_MC00_DEFINITIONS} ${PIC32CM_MC0027_COMPILE_DEFINITIONS}
            COMPILE_OPTIONS ${CORTEX_M0_COMMON_OPTIONS} ${CORTEX_M0_COMPILE_OPTIONS} -DBOARD_NAME_${name}
            INCLUDE_DIRECTORIES ${PIC32CM_MC00_INCLUDE_DIRS}
            LINK_OPTIONS ${CORTEX_M0_COMMON_OPTIONS} ${CORTEX_M0_LINK_OPTIONS}
            LINKER_SCRIPT pic32cm6408mc00032_flash
            SVD PIC32CM_MC0011
            OPTIMIZATION -Og

            OPENOCD_TARGET pic32cm

            ${ARGN}
    )
endfunction()
