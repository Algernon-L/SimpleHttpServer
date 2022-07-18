set(sql_SRCS
    SqlConnectionPool.cpp
    SqlOperation.cpp
)

add_library(mymuduo_sql ${sql_SRCS})
target_link_libraries(mymuduo_sql pthread)

install(TARGETS mymuduo_sql DESTINATION lib)
set(HEADERS
    SqlConnectionPool.h
    SqlOperation.h
)

install(FILES ${HEADERS} DESTINATION include/mymuduo/sql)

