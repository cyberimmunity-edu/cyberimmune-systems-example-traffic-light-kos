
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* EDL description of the LightsGPIO entity. */
#include <traffic_light/LightsGPIO.edl.h>

/* Description of the DiagnosticSystem interface used by the `LightsGPIO` entity. */
#include <traffic_light/IPing.idl.h>

#include <assert.h>

#define MSGS_NUM 1

/* Type of interface implementing object. */
typedef struct IModeImpl {
    struct traffic_light_IMode base;     /* Base interface of object */
    rtl_uint32_t step;                   /* Extra parameters */
} IModeImpl;

/* Mode method implementation. */
static nk_err_t FMode_impl(struct traffic_light_IMode *self,
                          const struct traffic_light_IMode_FMode_req *req,
                          const struct nk_arena *req_arena,
                          traffic_light_IMode_FMode_res *res,
                          struct nk_arena *res_arena)
{
    IModeImpl *impl = (IModeImpl *)self;
    /**
     * Increment value in control system request by
     * one step and include into result argument that will be
     * sent to the control system in the lights gpio response.
     */
    res->result = req->value + impl->step;
    return NK_EOK;
}

/**
 * IMode object constructor.
 * step is the number by which the input value is increased.
 */
static struct traffic_light_IMode *CreateIModeImpl(rtl_uint32_t step)
{
    /* Table of implementations of IMode interface methods. */
    static const struct traffic_light_IMode_ops ops = {
        .FMode = FMode_impl
    };

    /* Interface implementing object. */
    static struct IModeImpl impl = {
        .base = {&ops}
    };

    impl.step = step;

    return &impl.base;
}


/* Lights GPIO entry point. */
int main(void)
{
    NkKosTransport transport;
    ServiceId iid;

    /* Get lights gpio IPC handle of "lights_gpio_connection". */
    Handle handle = ServiceLocatorRegister("lights_gpio_connection", NULL, 0, &iid);
    assert(handle != INVALID_HANDLE);  

    /* Initialize transport to control system. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);
  

    /**
     * Prepare the structures of the request to the lights gpio entity: constant
     * part and arena. Because none of the methods of the lights gpio entity has
     * sequence type arguments, only constant parts of the
     * request and response are used. Arenas are effectively unused. However,
     * valid arenas of the request and response must be passed to
     * lights gpio transport methods (nk_transport_recv, nk_transport_reply) and
     * to the lights gpio method.
     */
    traffic_light_LightsGPIO_entity_req req;
    char req_buffer[traffic_light_LightsGPIO_entity_req_arena_size];
    struct nk_arena req_arena = NK_ARENA_INITIALIZER(req_buffer,
                                        req_buffer + sizeof(req_buffer));

    /* Prepare response structures: constant part and arena. */
    traffic_light_LightsGPIO_entity_res res;
    char res_buffer[traffic_light_LightsGPIO_entity_res_arena_size];
    struct nk_arena res_arena = NK_ARENA_INITIALIZER(res_buffer,
                                        res_buffer + sizeof(res_buffer));

    /**
     * Initialize mode component dispatcher. 3 is the value of the step,
     * which is the number by which the input value is increased.
     */
    traffic_light_CMode_component component;
    traffic_light_CMode_component_init(&component, CreateIModeImpl(0x1000000));
    /* Initialize lights gpio entity dispatcher. */
    traffic_light_LightsGPIO_entity entity;
    traffic_light_LightsGPIO_entity_init(&entity, &component);

    fprintf(stderr, "Hello I'm LightsGPIO\n");


    NkKosTransport transport2;
    struct traffic_light_IPing_proxy proxy2;
    /*static const nk_uint32_t tl_diagnostics = traffic_light_IPing_DiagnisticMsg;

    /*
     * Get the LightsGPIO IPC handle of the connection named
     * "lights_gpio_connection".
     */
    Handle handle2 = ServiceLocatorConnect("diagnostic_system_connection");
    assert(handle2 != INVALID_HANDLE);

    /* Initialize IPC transport for interaction with the lights gpio entity. */
    NkKosTransport_Init(&transport2, handle2, NK_NULL, 0);

    /**
     * Get Runtime Interface ID (RIID) for interface traffic_light.Mode.mode.
     * Here mode is the name of the traffic_light.Mode component instance,
     * traffic_light.Mode.mode is the name of the Mode interface implementation.
     */
    nk_iid_t riid2 = ServiceLocatorGetRiid(handle2, "diagnostics.ping");
    assert(riid2 != INVALID_RIID);

    /**
     * Initialize proxy object by specifying transport (&transport)
     * and lights gpio interface identifier (riid). Each method of the
     * proxy object will be implemented by sending a request to the lights gpio.
     */
    traffic_light_IPing_proxy_init(&proxy2, &transport2.base, riid2);

    /* Request and response structures */
    traffic_light_IPing_FPing_req req2;
    traffic_light_IPing_FPing_res res2;

    /* Dispatch loop implementation. */
    do
    {
        /* Flush request/response buffers. */
        nk_req_reset(&req);
        nk_arena_reset(&req_arena);
        nk_arena_reset(&res_arena);

        /* Wait for request to lights gpio entity. */
        if (nk_transport_recv(&transport.base,
                              &req.base_,
                              &req_arena) != NK_EOK) {
            fprintf(stderr, "nk_transport_recv error\n");
        } else {
            /**
             * Handle received request by calling implementation Mode_impl
             * of the requested Mode interface method.
             */
            traffic_light_LightsGPIO_entity_dispatch(&entity, &req.base_, &req_arena,
                                        &res.base_, &res_arena);
        }

        /* Send response. */
        if (nk_transport_reply(&transport.base,
                               &res.base_,
                               &res_arena) != NK_EOK) {
            fprintf(stderr, "nk_transport_reply error\n");
        }
        
        /* DiagnosticSystem Test loop. */
        static const nk_uint32_t tl_diagnostics = traffic_light_IPing_DiagnisticMsg;

        req2.value = req.lightsGpio_mode.FMode.value + tl_diagnostics;
        /**
        * Call Mode interface method.
        * Lights GPIO will be sent a request for calling Mode interface method
        * mode_comp.mode_impl with the value argument. Calling thread is locked
        * until a response is received from the lights gpio.
        */
        if (traffic_light_IPing_FPing(&proxy2.base, &req2, NULL, &res2, NULL) == rcOk)

        {
            /**
            * Print result value from response
            * (result is the output argument of the Mode method).
            */
            fprintf(stderr, "result2 = %0x\n", (int) res2.result);
            /**
            * Include received result value into value argument
            * to resend to lights gpio in next iteration.
            */
            req2.value = res2.result;

        }
        else
            fprintf(stderr, "Failed to call traffic_light.DiagnosticSystem.Ping()\n");

    }
    while (true);

    return EXIT_SUCCESS;
}
