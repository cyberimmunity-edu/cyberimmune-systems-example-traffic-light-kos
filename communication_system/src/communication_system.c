
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
#include <unistd.h> // read(), write(), close()
#include <strings.h> // bzero()

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* Description of the server interface used by the `client` entity. */
#include <traffic_light/IPing.idl.h>

#include <assert.h>
#include <json.h>
#include "include/response-parser.h"

#define DISCOVERING_IFACE_MAX   10
#define TIME_STEP_SEC           5
#define HOST_IP                 "10.0.2.2"
#define HOST_PORT               8081
#define NUM_RETRIES             10
#define MSG_BUF_SIZE            1024
#define MSG_CHUNK_BUF_SIZE      256
#define SA struct sockaddr

#define EXAMPLE_VALUE_TO_SEND 777

static const char LogPrefix[] = "[Connector]";


/* get traffic light configuration from the central server */
int get_traffic_light_configuration(void)
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "%s DEBUG: Socket creation failed...\n\n\n", LogPrefix);
        return EXIT_FAILURE;
    }
    else
        fprintf(stderr, "%s DEBUG: Socket successfully created..\n\n\n", LogPrefix);
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(HOST_IP);

    if ( servaddr.sin_addr.s_addr == INADDR_NONE ) {
        fprintf(stderr, "%s bad address!", LogPrefix);
    }

    servaddr.sin_port = htons(HOST_PORT);

    int res = -1;

    res = -1;
    for (int i = 0; res == -1 && i < NUM_RETRIES; i++)
    {
        sleep(1); // Wait some time for server be ready.
        res = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
    }

    // connect the client socket to server socket
    if (res != 0) {
        fprintf(stderr, "%s DEBUG: Connection with the server failed... %d\n\n\n", LogPrefix, res);
    }
    else
        fprintf(stderr, "%s DEBUG: Connected to the server..\n\n\n", LogPrefix);

    printf("preparing request..\n");

    char request_data[MSG_BUF_SIZE];
    char response_data[MSG_BUF_SIZE];
    char response_data_chunk[MSG_BUF_SIZE];
    int  request_len = 0;    
    size_t n;

    snprintf(request_data, MSG_CHUNK_BUF_SIZE,
        "GET /mode/112233 HTTP/1.1\r\n"
        "Host: 172.20.172.221:5765\r\n\r\n"
        // "Host-Agent: KOS\r\n"
        // "Accept: */*\r\n"
    );

    request_len = strlen(request_data);
    response_data[0] = 0;
    response_data_chunk[0] = 0;
    // fprintf(stderr, "%s, sending request: %s\n len: %d\n", LogPrefix, request_data, request_len);

   /// Write the request
    if (write(sockfd, request_data, request_len) >= 0)
    {
        fprintf(stderr, "%s request sent, reading response..\n", LogPrefix);
        /// Read the response
        while ((n = read(sockfd, response_data_chunk, MSG_BUF_SIZE)) > 0)
        {
            strcat(response_data, response_data_chunk);
            fprintf(stderr, "%s response data: \n%s\n", LogPrefix, response_data);
        }
    }
    fprintf(stderr, "%s read data: %s..\n", LogPrefix, response_data);
    int rc = parse_response(response_data);
    fprintf(stderr, "%s response parsing result: \n%d\n", LogPrefix, rc);

    // close the socket
    close(sockfd);

    return EXIT_SUCCESS;
}


/* Connector entity entry point. */
int main(int argc, const char *argv[])
{
    NkKosTransport transport;
    struct traffic_light_IPing_proxy proxy;
    int               i;
    int               socketfd;
    struct            ifconf conf;
    struct            ifreq iface_req[DISCOVERING_IFACE_MAX];
    struct            ifreq *ifr;
    struct sockaddr * sa;
    bool              is_network_available;

    fprintf(stderr, "%s Hello, I'm CommunicationSystem and about to start working\n", LogPrefix);

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
    Handle handle = ServiceLocatorConnect("control_system_connection_with_communication");
    assert(handle != INVALID_HANDLE);

    /* Initialize IPC transport for interaction with the server entity. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    /* Get Runtime Interface ID (RIID) for interface traffic_light.IPing.ping.
     * Here ping is the name of the traffic_light.IPing component instance,
     * traffic_light.IPing.ping is the name of the Ping interface implementation. */
    nk_iid_t riid = ServiceLocatorGetRiid(handle, "traffic_light.controls.ping");
    assert(riid != INVALID_RIID);

    /* Initialize proxy object by specifying transport (&transport)
     * and server interface identifier (riid). Each method of the
     * proxy object will be implemented by sending a request to the server. */
    traffic_light_IPing_proxy_init(&proxy, &transport.base, riid);

    /* Request and response structures */
    traffic_light_IPing_FPing_req req;
    traffic_light_IPing_FPing_res res;

    /* Test loop. */
    req.value = EXAMPLE_VALUE_TO_SEND;
    int rc = get_traffic_light_configuration();    
    fprintf(stderr, "%s Сonfiguration parsing status: %s\n", LogPrefix, rc == EXIT_SUCCESS ? "OK" : "FAILED");
    
    for (i = 0; i < 10; ++i)
    {
        /* Call Ping interface method.
         * Server will be sent a request for calling Ping interface method
         * ping_comp.ping_impl with the value argument. Calling thread is locked
         * until a response is received from the server. */
        if (traffic_light_IPing_FPing(&proxy.base, &req, NULL, &res, NULL) == rcOk)

        {
            /* Print result value from response
             * (result is the output argument of the Ping method). */
            fprintf(stderr, "%s result4 = %d\n", LogPrefix, (int) res.result);
            /* Include received result value into value argument
             * to resend to server in next iteration. */
            req.value = res.result;

        }
        else
            fprintf(stderr, "%s Failed to call traffic_light.IPing.FPing()\n", LogPrefix);
    }

    return EXIT_SUCCESS;
}
