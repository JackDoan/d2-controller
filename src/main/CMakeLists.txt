main_sources(COMMON_SRC
        main.c

        common/crc.c
        common/crc.h

        common/streambuf.c
        common/streambuf.h
#        common/string_light.c
#        common/string_light.h
#        common/time.c
#        common/time.h
        common/typeconversion.c
        common/typeconversion.h
#        common/uvarint.c
#        common/uvarint.h
        common/circular_queue.c
        common/circular_queue.h
        common/fport.c
        common/fport.h

        rx/frsky_crc.c
        rx/frsky_crc.h
#        rx/sbus.c
#        rx/sbus.h
#        rx/spektrum.c
#        rx/spektrum.h
#        rx/srxl2.c
#        rx/srxl2.h
#        rx/sumd.c
#        rx/sumd.h
#

        )

add_subdirectory(target)
