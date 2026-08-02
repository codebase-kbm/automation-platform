#define _GNU_SOURCE

#include "can.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

typedef struct
{
    int socket_fd;

} ap_socketcan_context_t;

/* -------------------------------------------------- */
/* Forward declarations                               */
/* -------------------------------------------------- */

static void socketcan_close(void *context);

/* -------------------------------------------------- */
/* Backend lifecycle                                  */
/* -------------------------------------------------- */

static void *socketcan_create(void)
{
    ap_socketcan_context_t *ctx;

    ctx = calloc(1, sizeof(*ctx));

    if (ctx == NULL)
        return NULL;

    ctx->socket_fd = -1;

    return ctx;
}


static void socketcan_destroy(void *context)
{
    ap_socketcan_context_t *ctx = context;

    if (ctx == NULL)
        return;

    socketcan_close(ctx);

    free(ctx);
}


/* -------------------------------------------------- */
/* Backend open / close                               */
/* -------------------------------------------------- */

static int socketcan_open(void *context,const ap_can_backend_config_t *config)
{
    ap_socketcan_context_t *ctx = context;

    struct sockaddr_can address;
    struct ifreq interface;

    if (ctx == NULL ||
        config == NULL ||
        config->interface == NULL)
    {
        return -1;
    }

    ctx->socket_fd =
        socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (ctx->socket_fd < 0)
    {
        fprintf(
            stderr,
            "SocketCAN: error opening CAN socket: %s\n",
            strerror(errno)
        );

        return -1;
    }

    memset(
        &interface,
        0,
        sizeof(interface)
    );

    strncpy(
        interface.ifr_name,
        config->interface,
        IFNAMSIZ - 1
    );

    interface.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(
            ctx->socket_fd,
            SIOCGIFINDEX,
            &interface) < 0)
    {
        fprintf(
            stderr,
            "SocketCAN: error getting interface index: %s\n",
            strerror(errno)
        );

        close(ctx->socket_fd);
        ctx->socket_fd = -1;

        return -1;
    }

    memset(
        &address,
        0,
        sizeof(address)
    );

    address.can_family  = AF_CAN;
    address.can_ifindex = interface.ifr_ifindex;

    if (bind(
            ctx->socket_fd,
            (struct sockaddr *)&address,
            sizeof(address)) < 0)
    {
        fprintf(
            stderr,
            "SocketCAN: error binding interface %s: %s\n",
            config->interface,
            strerror(errno)
        );

        close(ctx->socket_fd);
        ctx->socket_fd = -1;

        return -1;
    }

    /*
     * SocketCAN does not configure the CAN bitrate here.
     * The interface is configured externally.
     */

    return 0;
}


static void socketcan_close(void *context)
{
    ap_socketcan_context_t *ctx = context;

    if (ctx == NULL)
        return;

    if (ctx->socket_fd >= 0)
    {
        close(ctx->socket_fd);
        ctx->socket_fd = -1;
    }
}


/* -------------------------------------------------- */
/* Backend data transfer                              */
/* -------------------------------------------------- */

static int socketcan_send(
    void *context,
    const ap_can_frame_t *frame)
{
    ap_socketcan_context_t *ctx = context;
    struct can_frame can_frame;

    if (ctx == NULL ||
        frame == NULL)
    {
        return -1;
    }

    if (ctx->socket_fd < 0 ||
        frame->dlc > CAN_MAX_DLEN)
    {
        return -1;
    }

    memset(
        &can_frame,
        0,
        sizeof(can_frame)
    );

    can_frame.can_id  = frame->can_id;
    can_frame.can_dlc = frame->dlc;

    memcpy(
        can_frame.data,
        frame->data,
        frame->dlc
    );

    if (write(
            ctx->socket_fd,
            &can_frame,
            sizeof(can_frame)) != sizeof(can_frame))
    {
        return -1;
    }

    return 0;
}


static int socketcan_receive(
    void *context,
    ap_can_frame_t *frame)
{
    ap_socketcan_context_t *ctx = context;
    struct can_frame can_frame;

    if (ctx == NULL ||
        frame == NULL)
    {
        return -1;
    }

    if (ctx->socket_fd < 0)
        return -1;

    if (read(
            ctx->socket_fd,
            &can_frame,
            sizeof(can_frame)) != sizeof(can_frame))
    {
        return -1;
    }

    if (can_frame.can_dlc > CAN_MAX_DLEN)
        return -1;

    frame->can_id = can_frame.can_id;
    frame->dlc    = can_frame.can_dlc;

    memcpy(
        frame->data,
        can_frame.data,
        frame->dlc
    );

    return 0;
}


/* -------------------------------------------------- */
/* CAN Backend                                        */
/* -------------------------------------------------- */

const ap_can_backend_t ap_can_backend =
{
    .create  = socketcan_create,
    .destroy = socketcan_destroy,

    .open    = socketcan_open,
    .close   = socketcan_close,

    .send    = socketcan_send,
    .receive = socketcan_receive
};
