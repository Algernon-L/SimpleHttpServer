set(utils_SRCS
    CurrentThread.cpp
    Timestamp.cpp
)

add_library(mymuduo_utils ${utils_SRCS})
target_link_libraries(mymuduo_utils pthread)

install(TARGETS mymuduo_utils DESTINATION lib)
set(HEADERS
    Atomic.h
    copyable.h
    noncopyable.h
    Timestamp.h
)

install(FILES ${HEADERS} DESTINATION include/mymuduo/utils)

