main_sources(COMMON_SRC
        main.c

        helpers.c
        helpers.h

        rx/fport.c
        rx/fport.h
        rx/fport_dma.c
        rx/fport_dma.h
        rx/fport_stats.c
        rx/fport_stats.h

        rx/frsky_crc.c
        rx/frsky_crc.h
        )

add_subdirectory(target)
