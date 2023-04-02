main_sources(COMMON_SRC
        main.c

        helpers.c
        helpers.h

        rx/fport.c
        rx/fport.h
        rx/fport_helpers.c
        rx/fport_helpers.h

        rx/frsky_crc.c
        rx/frsky_crc.h
        )

add_subdirectory(target)
