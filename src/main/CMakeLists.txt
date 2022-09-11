main_sources(COMMON_SRC
        main.c

        common/streambuf.c
        common/streambuf.h

        common/typeconversion.c
        common/typeconversion.h
        common/circular_queue.c
        common/circular_queue.h

        rx/fport.c
        rx/fport.h

        rx/frsky_crc.c
        rx/frsky_crc.h
        )

add_subdirectory(target)
