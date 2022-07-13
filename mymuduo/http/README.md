# HttpServer 工作流程

1.向HttpServer注册回调函数，设置路由。
2.onConnection()回调，设置消息类型为HttpContext
3.epoll_wait -> handleRead() -> MessageCallback_ -> onMessage()回调
4.onMessage -> parseRequest 解析请求（请求行、请求头、请求体）
5.解析完成，调用onRequest()