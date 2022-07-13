set(http_SRCS
    HttpContext.cpp
    HttpResponse.cpp
    HttpServer.cpp
)

add_library(mymuduo_http ${http_SRCS})
target_link_libraries(mymuduo_http pthread)

install(TARGETS mymuduo_http DESTINATION lib)
set(HEADERS
    HttpContext.h
    HttpRequest.h
    HttpResponse.h
    HttpServer.h
)

install(FILES ${HEADERS} DESTINATION include/mymuduo/http)

