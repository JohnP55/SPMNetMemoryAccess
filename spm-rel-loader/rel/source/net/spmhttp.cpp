#include <msl/string.h>
#include <spm/memory.h>
#include <wii/os/OSError.h>

#include "spmhttp.h"
#include "network.h"
#include "core_http_client.h"

int32_t recv(NetworkContext_t * pNetworkContext, void * pBuffer, size_t bytesToRecv) {
    return Mynet_recv(pNetworkContext->sockfd, pBuffer, bytesToRecv, 0);
}

int32_t send(NetworkContext_t * pNetworkContext, const void * pBuffer, size_t bytesToSend) {
    return Mynet_send(pNetworkContext->sockfd, pBuffer, bytesToSend, 0);
}

void HTTPFree(HTTPResponse_t* response) {
    if (response != nullptr) {
        if (response->pBuffer != nullptr) {
            delete response->pBuffer;
            response->pBuffer = nullptr;
        }
        response = {0};
    }
}

/// @brief Sends an HTTP get request. You must call HTTPFree on the returned response.
/// @param host The host (api.example.org)
/// @param port The port
/// @param path The path (/api/v1/)
/// @param response An empty HttpResponse. Its buffer will be set here, so you must free it.
/// @return A HTTPResponse. You must call HTTPFree on it when you're done using it.
HTTPStatus_t HTTPGet(const char* host, int port, const char* path, HTTPResponse_t* response) {
    HTTPStatus_t httpStatus = HTTPSuccess;

    httpStatus = HTTPSendRequest(host, port, path, HTTP_METHOD_GET, nullptr, (size_t)0, response);
    if (httpStatus != HTTPStatus::HTTPSuccess) {
        wii::os::OSReport("HTTPSendRequest failed with HTTPStatus %d\n", httpStatus);
    }

    return httpStatus;
}

/// @brief Sends a request over the HTTP protocol
/// @param host The host (api.example.org)
/// @param port The port
/// @param path The path (/api/v1/)
/// @param method The HTTP method (GET, POST, HEAD, and PUT are supported)
/// @param bodyBuffer The body of the request
/// @param bodyBufferSize The size of the body
/// @param response An empty initialized response with a buffer. The buffer will be filled.
/// @return An HTTPStatus code as defined in core_http_client.h
HTTPStatus_t HTTPSendRequest(const char* host, int port, const char* path, const char* method, u8* bodyBuffer, size_t bodyBufferSize, HTTPResponse_t* response) {
    HTTPStatus_t httpStatus = HTTPSuccess;

    struct hostent *server;
    struct sockaddr_in serv_addr;

    u32 sockfd = Mynet_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        wii::os::OSReport("ERROR opening socket\n");
        return HTTPStatus::HTTPNetworkError;
    }

    server = Mynet_gethostbyname((char*)host);
    if (server == NULL) {
        wii::os::OSReport("ERROR no such host\n");
        Mynet_close(sockfd);
        return HTTPStatus::HTTPNetworkError;
    }

    msl::string::memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    msl::string::memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    if (Mynet_connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        wii::os::OSReport("ERROR connecting\n");
        Mynet_close(sockfd);
        return HTTPStatus::HTTPNetworkError;
    }

    NetworkContext_t netctx = {.sockfd = sockfd};
    TransportInterface_t ti = {
        .recv=recv,
        .send=send,
        .pNetworkContext=&netctx
    };

    u8 requestHeaderBuffer[REQUEST_BUF_SIZE] = {0};
    HTTPRequestHeaders_t requestHeaders = {
        .pBuffer = requestHeaderBuffer,
        .bufferLen = REQUEST_BUF_SIZE
    };

    HTTPRequestInfo_t requestInfo = {
        .pMethod = method,
        .methodLen = msl::string::strlen(method),
        .pPath = path,
        .pathLen = msl::string::strlen(path),
        .pHost = host,
        .hostLen = msl::string::strlen(host)
    };

    httpStatus = HTTPClient_InitializeRequestHeaders(&requestHeaders, &requestInfo);
    wii::os::OSReport("HTTPStatus after InitializeRequestHeaders: %d\n", httpStatus);

    u8* responseBuffer = new u8[RESPONSE_BUF_SIZE];

    response->pBuffer = responseBuffer;
    response->bufferLen = RESPONSE_BUF_SIZE;
    
    httpStatus = HTTPClient_Send(&ti, &requestHeaders, bodyBuffer, bodyBufferSize, response, 0);
    Mynet_close(sockfd);

    return httpStatus;
}
