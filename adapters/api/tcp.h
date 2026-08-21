#ifndef AP_TCP_H
#define AP_TCP_H

#include <stddef.h>
#include <stdint.h>

#include "ap_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TCP backend interface.
 *
 * The backend provides a platform-independent TCP stream interface.
 * The Peer protocol is not part of this API.
 */
typedef struct ap_tcp_backend
{
    /**
     * @brief Create TCP backend context.
     *
     * @param context Receives backend context.
     *
     * @return AP_OK on success or an appropriate error code.
     */
    ap_result_t (*create)(void **context);

    /**
     * @brief Destroy TCP backend context.
     *
     * @param context Backend context.
     */
    void (*destroy)(void *context);

    /**
     * @brief Start listening for incoming TCP connections.
     *
     * @param context Backend context.
     * @param address Local bind address.
     * @param port Local TCP port.
     * @param backlog Listen backlog.
     *
     * @return AP_OK on success or an appropriate error code.
     */
    ap_result_t (*listen)(void *context,const char *address,uint16_t port,int backlog);

    /**
     * @brief Stop listening.
     *
     * @param context Backend context.
     */
    void (*close)(void *context);

    /**
     * @brief Accept an incoming TCP connection.
     *
     * @param context Backend context.
     * @param connection Receives connection context.
     * @param remote_address Receives remote IPv4 address.
     * @param remote_address_size Size of remote address buffer.
     * @param remote_port Receives remote TCP port.
     *
     * @return AP_OK on success or an appropriate error code.
     */
    ap_result_t (*accept)(
        void *context,
        void **connection,
        uint8_t remote_address[4],
        uint16_t *remote_port
    );

    /**
     * @brief Close a TCP connection.
     *
     * @param context Backend context.
     * @param connection Connection context.
     */
    void (*disconnect)(void *context,void *connection);

    /**
     * @brief Receive bytes from a TCP connection.
     *
     * TCP is a stream. The backend may return fewer bytes than requested.
     *
     * @param context Backend context.
     * @param connection Connection context.
     * @param buffer Receive buffer.
     * @param buffer_size Size of receive buffer.
     * @param received Receives number of bytes received.
     *
     * @return AP_OK on successful reception,
     *         AP_NOT_FOUND if no data is currently available,
     *         or an appropriate error code.
     */
    ap_result_t (*receive)(
        void *context,
        void *connection,
        uint8_t *buffer,
        size_t buffer_size,
        size_t *received
    );

    /**
     * @brief Send bytes over a TCP connection.
     *
     * TCP is a stream. The backend may send fewer bytes than requested.
     *
     * @param context Backend context.
     * @param connection Connection context.
     * @param buffer Data to send.
     * @param buffer_size Number of bytes to send.
     * @param sent Receives number of bytes sent.
     *
     * @return AP_OK on success or an appropriate error code.
     */
    ap_result_t (*send)(
        void *context,
        void *connection,
        const uint8_t *buffer,
        size_t buffer_size,
        size_t *sent
    );

} ap_tcp_backend_t;

/**
 * @brief Platform TCP backend.
 */
extern const ap_tcp_backend_t ap_tcp_backend;

#ifdef __cplusplus
}
#endif

#endif /* AP_TCP_H */