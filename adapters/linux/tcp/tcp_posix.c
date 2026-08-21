#include "tcp.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct
{
    int listen_fd;

} ap_tcp_posix_context_t;


typedef struct
{
    int fd;

} ap_tcp_posix_connection_t;


/*
 * --------------------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------------------
 */

static ap_result_t set_nonblocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0)
        return AP_ERROR_SOCKET_OPTION;

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return AP_ERROR_SOCKET_OPTION;

    return AP_OK;
}


/*
 * --------------------------------------------------------------------------
 * Backend
 * --------------------------------------------------------------------------
 */

static ap_result_t tcp_create(void **context)
{
    ap_tcp_posix_context_t *ctx;

    if (context == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    ctx = calloc(1, sizeof(*ctx));

    if (ctx == NULL)
        return AP_ERROR_OUT_OF_MEMORY;

    ctx->listen_fd = -1;

    *context = ctx;

    return AP_OK;
}


static void tcp_destroy(void *context)
{
    ap_tcp_posix_context_t *ctx;

    ctx = context;

    if (ctx == NULL)
        return;

    if (ctx->listen_fd >= 0)
        close(ctx->listen_fd);

    free(ctx);
}


static ap_result_t tcp_listen(
    void *context,
    const char *address,
    uint16_t port,
    int backlog)
{
    ap_tcp_posix_context_t *ctx;
    struct sockaddr_in addr;
    int fd;
    int reuse = 1;

    if (context == NULL || address == NULL)
        return AP_ERROR_INVALID_ARGUMENT;

    ctx = context;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
        return AP_ERROR_SOCKET_CREATE;

    if (setsockopt(
            fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)) < 0)
    {
        close(fd);
        return AP_ERROR_SOCKET_OPTION;
    }

    if (set_nonblocking(fd) != AP_OK)
    {
        close(fd);
        return AP_ERROR_SOCKET_OPTION;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &addr.sin_addr) != 1)
    {
        close(fd);
        return AP_ERROR_SOCKET_RESOLVE;
    }

    if (bind(
            fd,
            (struct sockaddr *)&addr,
            sizeof(addr)) < 0)
    {
        close(fd);
        return AP_ERROR_SOCKET_BIND;
    }

    if (listen(fd, backlog) < 0)
    {
        close(fd);
        return AP_ERROR_SOCKET_LISTEN;
    }

    ctx->listen_fd = fd;

    return AP_OK;
}


static void tcp_close(void *context)
{
    ap_tcp_posix_context_t *ctx;

    ctx = context;

    if (ctx == NULL)
        return;

    if (ctx->listen_fd >= 0)
    {
        close(ctx->listen_fd);
        ctx->listen_fd = -1;
    }
}


static ap_result_t tcp_accept(
    void *context,
    void **connection,
    uint8_t remote_address[4],
    uint16_t *remote_port)
{
    ap_tcp_posix_context_t *ctx;
    ap_tcp_posix_connection_t *conn;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int fd;

    if (context == NULL ||
        connection == NULL ||
        remote_address == NULL ||
        remote_port == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    *connection = NULL;

    ctx = context;

    if (ctx->listen_fd < 0)
        return AP_ERROR_NOT_INITIALIZED;

    addr_len = sizeof(addr);

    fd = accept(
        ctx->listen_fd,
        (struct sockaddr *)&addr,
        &addr_len);

    if (fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return AP_OK;

        if (errno == EINTR)
            return AP_OK;

        return AP_ERROR_SOCKET_ACCEPT;
    }

    if (set_nonblocking(fd) != AP_OK)
    {
        close(fd);
        return AP_ERROR_SOCKET_OPTION;
    }

    conn = calloc(1, sizeof(*conn));

    if (conn == NULL)
    {
        close(fd);
        return AP_ERROR_OUT_OF_MEMORY;
    }

    conn->fd = fd;

    memcpy(
        remote_address,
        &addr.sin_addr,
        sizeof(addr.sin_addr));

    *remote_port = ntohs(addr.sin_port);

    *connection = conn;

    return AP_OK;
}


static void tcp_disconnect(
    void *context,
    void *connection)
{
    ap_tcp_posix_connection_t *conn;

    (void)context;

    conn = connection;

    if (conn == NULL)
        return;

    if (conn->fd >= 0)
        close(conn->fd);

    free(conn);
}


static ap_result_t tcp_receive(
    void *context,
    void *connection,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *received)
{
    ap_tcp_posix_connection_t *conn;
    ssize_t result;

    (void)context;

    if (connection == NULL ||
        buffer == NULL ||
        received == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    *received = 0;

    if (buffer_size == 0)
        return AP_ERROR_INVALID_SIZE;

    conn = connection;

    result = recv(
        conn->fd,
        buffer,
        buffer_size,
        0);

    if (result > 0)
    {
        *received = (size_t)result;
        return AP_OK;
    }

    if (result == 0)
        return AP_ERROR_CONNECTION;

    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return AP_OK;

    if (errno == EINTR)
        return AP_OK;

    return AP_ERROR_SOCKET_RECEIVE;
}


static ap_result_t tcp_send(
    void *context,
    void *connection,
    const uint8_t *buffer,
    size_t buffer_size,
    size_t *sent)
{
    ap_tcp_posix_connection_t *conn;
    ssize_t result;

    (void)context;

    if (connection == NULL ||
        buffer == NULL ||
        sent == NULL)
    {
        return AP_ERROR_INVALID_ARGUMENT;
    }

    *sent = 0;

    if (buffer_size == 0)
        return AP_OK;

    conn = connection;

    result = send(
        conn->fd,
        buffer,
        buffer_size,
        MSG_NOSIGNAL);

    if (result > 0)
    {
        *sent = (size_t)result;
        return AP_OK;
    }

    if (result == 0)
        return AP_OK;

    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return AP_OK;

    if (errno == EINTR)
        return AP_OK;

    return AP_ERROR_SOCKET_SEND;
}


/*
 * --------------------------------------------------------------------------
 * Backend instance
 * --------------------------------------------------------------------------
 */

const ap_tcp_backend_t ap_tcp_backend =
{
    .create      = tcp_create,
    .destroy     = tcp_destroy,
    .listen      = tcp_listen,
    .close       = tcp_close,
    .accept      = tcp_accept,
    .disconnect  = tcp_disconnect,
    .receive     = tcp_receive,
    .send        = tcp_send
};