
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* EDL description of the ControlSystem entity. */
#include <traffic_light/ControlSystem.edl.h>

/* Description of the LightGPIO interface used by the `ControlSystem` entity. */
#include <traffic_light/IMode.idl.h>

#include <assert.h>

#define MODES_NUM 18

/* Type of interface implementing object. */
typedef struct IPingImpl {
    struct traffic_light_IPing base;     /* Base interface of object */
    rtl_uint32_t step;                   /* Extra parameters */
} IPingImpl;

/* Ping method implementation. */
static nk_err_t FPing_impl(struct traffic_light_IPing *self,
                          const struct traffic_light_IPing_FPing_req *req,
                          const struct nk_arena *req_arena,
                          traffic_light_IPing_FPing_res *res,
                          struct nk_arena *res_arena)
{
    IPingImpl *impl = (IPingImpl *)self;
    /**
     * Increment value in control system request by
     * one step and include into result argument that will be
     * sent to the control system in the lights gpio response.
     */
    res->result = req->value + impl->step;
    return NK_EOK;
}

/**
 * IPing object constructor.
 * step is the number by which the input value is increased.
 */
static struct traffic_light_IPing *CreateIPingImpl(rtl_uint32_t step)
{
    /* Table of implementations of IPing interface methods. */
    static const struct traffic_light_IPing_ops ops = {
        .FPing = FPing_impl
    };

    /* Interface implementing object. */
    static struct IPingImpl impl = {
        .base = {&ops}
    };

    impl.step = step;

    return &impl.base;
}

/* Control system entity entry point. (int argc, const char *argv[]) */
int main(void)
{
    NkKosTransport transport;
    struct traffic_light_IMode_proxy proxy;
    int i;
    static const nk_uint32_t tl_Modes[MODES_NUM] = {
        traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction2Red,
        traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction1Yellow + traffic_light_IMode_Direction2Red,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction2Red,
        traffic_light_IMode_Direction1Yellow + traffic_light_IMode_Direction2Red,
        traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction2Yellow + traffic_light_IMode_Direction2Red,
        traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction2Green,
        traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction2Yellow,
        traffic_light_IMode_Direction1Yellow + traffic_light_IMode_Direction1Blink + traffic_light_IMode_Direction2Yellow + traffic_light_IMode_Direction2Blink,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction2Green, // <-- try to forbid this via security policies
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Blink + traffic_light_IMode_Direction2Green,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Yellow + traffic_light_IMode_Direction2Green,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Blink + traffic_light_IMode_Direction2Green + traffic_light_IMode_Direction2Yellow,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Yellow + traffic_light_IMode_Direction2Green + traffic_light_IMode_Direction2Yellow,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Blink + traffic_light_IMode_Direction2Green + traffic_light_IMode_Direction2Blink,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction2Green,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Red + traffic_light_IMode_Direction2Green + traffic_light_IMode_Direction2Red,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction2Red + traffic_light_IMode_Direction2Blink,
        traffic_light_IMode_Direction1Green + traffic_light_IMode_Direction1Yellow + traffic_light_IMode_Direction1Blink + traffic_light_IMode_Direction2Blink + traffic_light_IMode_Direction2Red
};

    fprintf(stderr, "Hello I'm ControlSystem\n");

    /**
     * Get the LightsGPIO IPC handle of the connection named
     * "lights_gpio_connection".
     */
    Handle handle = ServiceLocatorConnect("lights_gpio_connection");
    assert(handle != INVALID_HANDLE);

    /* Initialize IPC transport for interaction with the lights gpio entity. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    /**
     * Get Runtime Interface ID (RIID) for interface traffic_light.IMode.mode.
     * Here mode is the name of the traffic_light.IMode component instance,
     * traffic_light.IMode.mode is the name of the Mode interface implementation.
     */
    nk_iid_t riid = ServiceLocatorGetRiid(handle, "lightsGpio.mode");
    assert(riid != INVALID_RIID);

    /**
     * Initialize proxy object by specifying transport (&transport)
     * and lights gpio interface identifier (riid). Each method of the
     * proxy object will be implemented by sending a request to the lights gpio.
     */
    traffic_light_IMode_proxy_init(&proxy, &transport.base, riid);

    /* Request and response structures */
    traffic_light_IMode_FMode_req req;
    traffic_light_IMode_FMode_res res;

    fprintf(stderr, "Hello I'm ControlSystem\n");

    NkKosTransport transport2;
    ServiceId iid2;

    /* Get ControlSystem IPC handle of "ControlSystem_connection". */
    Handle handle2 = ServiceLocatorRegister("control_system_connection_with_communication", NULL, 0, &iid2);
    assert(handle2 != INVALID_HANDLE);

    /* Initialize transport to client. */
    NkKosTransport_Init(&transport2, handle2, NK_NULL, 0);

    /* Prepare the structures of the request to the ControlSystem entity: constant
     * part and arena. Because none of the methods of the ControlSystem entity has
     * sequence type arguments, only constant parts of the
     * request and response are used. Arenas are effectively unused. However,
     * valid arenas of the request and response must be passed to
     * ControlSystem transport methods (nk_transport_recv, nk_transport_reply) and
     * to the ControlSystem_entity_dispatch method. */
    traffic_light_ControlSystem_entity_req req2;
    char req_buffer[traffic_light_ControlSystem_entity_req_arena_size];
    struct nk_arena req_arena = NK_ARENA_INITIALIZER(req_buffer, req_buffer + sizeof(req_buffer));

    /* Prepare response structures: constant part and arena. */
    traffic_light_ControlSystem_entity_res res2;
    char res_buffer[traffic_light_ControlSystem_entity_res_arena_size];
    struct nk_arena res_arena = NK_ARENA_INITIALIZER(res_buffer, res_buffer + sizeof(res_buffer));

    /* Initialize Ping component dispatcher. 3 is the value of the step,
     * which is the number by which the input value is increased. */
    traffic_light_CPing_component component;
    traffic_light_CPing_component_init(&component, CreateIPingImpl(0x1000000));

    /* Initialize ControlSystem entity dispatcher. */
    traffic_light_ControlSystem_entity entity;
    traffic_light_ControlSystem_entity_init(&entity, &component);

    fprintf(stderr, "Hello I'm ControlSystem (Server)\n");

    /* Dispatch loop implementation. */
    do
    {
        /* Flush request/response buffers. */
        nk_req_reset(&req2);
        nk_arena_reset(&req_arena);
        nk_arena_reset(&res_arena);

        /* Wait for request to ControlSystem entity. */
        if (nk_transport_recv(&transport2.base, &req2.base_, &req_arena) != NK_EOK) {
            fprintf(stderr, "nk_transport_recv error\n");
        } else {
            /* Handle received request by calling implementation Ping
             * of the requested FPing interface method. */
            traffic_light_ControlSystem_entity_dispatch(&entity, &req2.base_, &req_arena, &res2.base_, &res_arena);
        }

        /* Send response. */
        if (nk_transport_reply(&transport2.base, &res2.base_, &res_arena) != NK_EOK) {
            fprintf(stderr, "nk_transport_reply error\n");
        }

        
        /* Test loop. */
        req.value = 0;
        for (i = 0; i < MODES_NUM; i++)
        {
            req.value = tl_Modes[i];
            /**
            * Call Ping interface method.
            * Lights GPIO will be sent a request for calling Ping interface method
            * Ping_comp.Ping_impl with the value argument. Calling thread is locked
            * until a response is received from the lights gpio.
            */
            if (traffic_light_IMode_FMode(&proxy.base, &req, NULL, &res, NULL) == rcOk)
            
            {
                /**
                * Print result value from response
                * (result is the output argument of the Ping method).
                */
                fprintf(stderr, "result = %0x\n", (int) res.result);
                /**
                * Include received result value into value argument
                * to resend to lights gpio in next iteration.
                */
                req.value = res.result;

            }
            else
                fprintf(stderr, "Failed to call traffic_light.IMode.FMode()\n");
        }

    }
    while (true);

    return EXIT_SUCCESS;
}