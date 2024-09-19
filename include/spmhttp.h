#pragma once

#include "core_http_client.h"

#define REQUEST_BUF_SIZE 1024
#define RESPONSE_BUF_SIZE 2048

extern "C" {

typedef struct NetworkContext {
    u32 sockfd;
} NetworkContext_t;

int32_t recv(NetworkContext_t *, void *, size_t);
int32_t send(NetworkContext_t *, const void *, size_t);

void HTTPFree(HTTPResponse_t* response);
HTTPStatus_t HTTPGet(const char* host, int port, const char* path, HTTPResponse_t* response);
HTTPStatus_t HTTPSendRequest(const char* host, int port, const char* path, const char* method, u8* bodyBuffer, size_t bodyBufferSize, HTTPResponse_t* response);

}