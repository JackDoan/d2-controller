set(CORTEX_M0_COMMON_OPTIONS
    -mthumb
    -mcpu=cortex-m0plus
    -march=armv6-m
    -mfloat-abi=softfp
    -Wdouble-promotion
)

set(CORTEX_M0_COMPILE_OPTIONS
)

set(CORTEX_M0_LINK_OPTIONS
)

set(CORTEX_M0_DEFINITIONS
    __FPU_PRESENT=0
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    UNALIGNED_SUPPORT_DISABLE
)
