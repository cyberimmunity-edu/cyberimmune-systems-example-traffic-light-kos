#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* Description of the control_system interface used by the `client` entity. */
#include <traffic_light/Ping.idl.h>

#include <assert.h>

#define EXAMPLE_VALUE_TO_SEND 777

/* Client entity entry point. */
int main(int argc, const char *argv[])
{
    NkKosTransport transport;
    struct traffic_light_Ping_proxy proxy;
    int i;

    fprintf(stderr, "Hello I'm Connection\n");

    /* Get the client IPC handle of the connection named
     * "control_system_connection". */
    Handle handle = ServiceLocatorConnect("control_system_connection");
    assert(handle != INVALID_HANDLE);

    /* Initialize IPC transport for interaction with the control_system entity. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    /* Get Runtime Interface ID (RIID) for interface traffic_light.Ping.ping.
     * Here ping is the name of the traffic_light.Ping component instance,
     * traffic_light.Ping.ping is the name of the Ping interface implementation. */
    nk_iid_t riid = ServiceLocatorGetRiid(handle, "traffic_light.Ping.ping");
    assert(riid != INVALID_RIID);

    /* Initialize proxy object by specifying transport (&transport)
     * and control_system interface identifier (riid). Each method of the
     * proxy object will be implemented by sending a request to the control_system. */
    traffic_light_Ping_proxy_init(&proxy, &transport.base, riid);

    /* Request and response structures */
    traffic_light_Ping_Ping_req req;
    traffic_light_Ping_Ping_res res;

    /* Test loop. */
    req.value = EXAMPLE_VALUE_TO_SEND;
    for (i = 0; i < 10; ++i)
    {
        /* Call Ping interface method.
         * control_system will be sent a request for calling Ping interface method
         * ping_comp.ping_impl with the value argument. Calling thread is locked
         * until a response is received from the control_system. */
        if (traffic_light_Ping_Ping(&proxy.base, &req, NULL, &res, NULL) == rcOk)

        {
            /* Print result value from response
             * (result is the output argument of the Ping method). */
            fprintf(stderr, "result = %d\n", (int) res.result);
            /* Include received result value into value argument
             * to resend to control_system in next iteration. */
            req.value = res.result;

        }
        else
            fprintf(stderr, "Failed to call traffic_light.Ping.Ping()\n");
    }

    return EXIT_SUCCESS;
}
