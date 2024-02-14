
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <rtl/stdbool.h>
#include <kos_net.h>

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* Description of the server interface used by the `client` entity. */
#include <echo/Ping.idl.h>

#include <assert.h>

#define DISCOVERING_IFACE_MAX   10
#define TIME_STEP_SEC           5

#define EXAMPLE_VALUE_TO_SEND 777

static const char LogPrefix[] = "[Connector]";

/* Connector entity entry point. */
int main(int argc, const char *argv[])
{
    NkKosTransport transport;
    struct echo_Ping_proxy proxy;
    int               i;
    int               socketfd;
    struct            ifconf conf;
    struct            ifreq iface_req[DISCOVERING_IFACE_MAX];
    struct            ifreq *ifr;
    struct sockaddr * sa;    
    bool              is_network_available;

    fprintf(stderr, "%s Hello, I'm about to start working\n", LogPrefix);

    is_network_available = wait_for_network();

    fprintf(stderr, "%s Network status: %s\n", LogPrefix, is_network_available ? "ok" : "not ok");
    fprintf(stderr, "%s Opening socket...\n", LogPrefix);

    socketfd = socket(AF_ROUTE, SOCK_RAW, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "\n%s cannot create socket\n", LogPrefix);
        return EXIT_FAILURE;
    }

    conf.ifc_len = sizeof(iface_req);
    conf.ifc_buf = (__caddr_t) iface_req;

    if (ioctl(socketfd,SIOCGIFCONF,&conf) < 0)
    {
        fprintf(stderr, "%s ioctl call failed\n", LogPrefix);
        close(socketfd);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "%s Discovering interfaces...\n", LogPrefix);

    for (i = 0; i < conf.ifc_len / sizeof(iface_req[0]); i ++)
    {
        ifr = &conf.ifc_req[i];
        sa = (struct sockaddr *) &ifr->ifr_addr;

        if (sa->sa_family == AF_INET)
        {
            struct sockaddr_in *sin = (struct sockaddr_in*) &ifr->ifr_addr;

            fprintf(stderr, "%s %s %s\n",
                    LogPrefix, ifr->ifr_name, inet_ntoa(sin->sin_addr));
        }
    }

    fprintf(stderr, "%s Network check up: OK\n", LogPrefix);

    /* Get the client IPC handle of the connection named
     * "server_connection". */
    Handle handle = ServiceLocatorConnect("server_connection");
    assert(handle != INVALID_HANDLE);

    /* Initialize IPC transport for interaction with the server entity. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    /* Get Runtime Interface ID (RIID) for interface echo.Ping.ping.
     * Here ping is the name of the echo.Ping component instance,
     * echo.Ping.ping is the name of the Ping interface implementation. */
    nk_iid_t riid = ServiceLocatorGetRiid(handle, "echo.Ping.ping");
    assert(riid != INVALID_RIID);

    /* Initialize proxy object by specifying transport (&transport)
     * and server interface identifier (riid). Each method of the
     * proxy object will be implemented by sending a request to the server. */
    echo_Ping_proxy_init(&proxy, &transport.base, riid);

    /* Request and response structures */
    echo_Ping_Ping_req req;
    echo_Ping_Ping_res res;

    /* Test loop. */
    req.value = EXAMPLE_VALUE_TO_SEND;
    for (i = 0; i < 10; ++i)
    {
        /* Call Ping interface method.
         * Server will be sent a request for calling Ping interface method
         * ping_comp.ping_impl with the value argument. Calling thread is locked
         * until a response is received from the server. */
        if (echo_Ping_Ping(&proxy.base, &req, NULL, &res, NULL) == rcOk)

        {
            /* Print result value from response
             * (result is the output argument of the Ping method). */
            fprintf(stderr, "%s result = %d\n", LogPrefix, (int) res.result);
            /* Include received result value into value argument
             * to resend to server in next iteration. */
            req.value = res.result;

        }
        else
            fprintf(stderr, "%s Failed to call echo.Ping.Ping()\n", LogPrefix);
    }

    return EXIT_SUCCESS;
}
