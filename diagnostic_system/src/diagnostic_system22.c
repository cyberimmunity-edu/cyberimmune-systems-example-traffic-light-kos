
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Files required for transport initialization. */
#include <coresrv/nk/transport-kos.h>
#include <coresrv/sl/sl_api.h>

/* EDL description of the DiagnosticSystem entity. */
#include <traffic_light/DiagnosticSystem.edl.h>

#include <assert.h>

/* Type of interface implementing object. */
typedef struct IModeImpl {
    struct traffic_light_IMode base;     // Base interface of object
    rtl_uint32_t step;             // Extra parameters
} IModeImpl;

/* Mode method implementation. */
static nk_err_t FMode_impl(struct traffic_light_IMode *self,
                          const traffic_light_IMode_FMode_req *req,
                          const struct nk_arena* req_arena,
                          traffic_light_IMode_FMode_res* res,
                          struct nk_arena* res_arena)
{
    IModeImpl *impl = (IModeImpl *)self;
    /* Increment value in client request by
     * one step and include into result argument that will be
     * sent to the client in the DiagnosticSystem response. */
    res->result = req->value + impl->step;
    return NK_EOK;
}

/* IMode object constructor.
 * step is the number by which the input value is increased. */
static struct traffic_light_IMode *CreateIModeImpl(int step)
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

/* DiagnosticSystem entry point. */
int main(void)
{
    fprintf(stderr, "Hello I'm DiagnosticSystem00\n");
    NkKosTransport transport;
    ServiceId iid;

    fprintf(stderr, "Hello I'm DiagnosticSystem0\n");

    /* Get DiagnosticSystem IPC handle of "DiagnosticSystem_connection". */
    Handle handle = ServiceLocatorRegister("diagnostic_system_connection", NULL, 0, &iid);
    assert(handle != INVALID_HANDLE);

    fprintf(stderr, "Hello I'm DiagnosticSystem1\n");

    /* Initialize transport to client. */
    NkKosTransport_Init(&transport, handle, NK_NULL, 0);

    fprintf(stderr, "Hello I'm DiagnosticSystem2\n");

    /* Prepare the structures of the request to the DiagnosticSystem entity: constant
     * part and arena. Because none of the methods of the DiagnosticSystem entity has
     * sequence type arguments, only constant parts of the
     * request and response are used. Arenas are effectively unused. However,
     * valid arenas of the request and response must be passed to
     * DiagnosticSystem transport methods (nk_transport_recv, nk_transport_reply) and
     * to the DiagnosticSystem_entity_dispatch method. */
    traffic_light_DiagnosticSystem_entity_req req;
    char req_buffer[traffic_light_DiagnosticSystem_entity_req_arena_size];
    struct nk_arena req_arena = NK_ARENA_INITIALIZER(req_buffer, req_buffer + sizeof(req_buffer));

    fprintf(stderr, "Hello I'm DiagnosticSystem3\n");
    /* Prepare response structures: constant part and arena. */
    traffic_light_DiagnosticSystem_entity_res res;
    char res_buffer[traffic_light_DiagnosticSystem_entity_res_arena_size];
    struct nk_arena res_arena = NK_ARENA_INITIALIZER(res_buffer, res_buffer + sizeof(res_buffer));

    fprintf(stderr, "Hello I'm DiagnosticSystem4\n");
    /* Initialize Mode component dispatcher. 3 is the value of the step,
     * which is the number by which the input value is increased. */
    traffic_light_CMode_component component;
    traffic_light_CMode_component_init(&component, CreateIModeImpl(0x1000000));

    fprintf(stderr, "Hello I'm DiagnosticSystem5\n");
    /* Initialize DiagnosticSystem entity dispatcher. */
    traffic_light_DiagnosticSystem_entity entity;
    traffic_light_DiagnosticSystem_entity_init(&entity, &component);

    fprintf(stderr, "Hello I'm DiagnosticSystem\n");

    /* Dispatch loop implementation. */
    do
    {
        /* Flush request/response buffers. */
        nk_req_reset(&req);
        nk_arena_reset(&req_arena);
        nk_arena_reset(&res_arena);

        /* Wait for request to DiagnosticSystem entity. */
        if (nk_transport_recv(&transport.base, &req.base_, &req_arena) != NK_EOK) {
            fprintf(stderr, "nk_transport_recv error\n");
        } else {
            /* Handle received request by calling implementation Mode
             * of the requested Mode interface method. */
            traffic_light_DiagnosticSystem_entity_dispatch(&entity, &req.base_, &req_arena, &res.base_, &res_arena);
        }


        /* Send response. */
        if (nk_transport_reply(&transport.base, &res.base_, &res_arena) != NK_EOK) {
            fprintf(stderr, "nk_transport_reply error\n");
        }
    }
    while (true);

    return EXIT_SUCCESS;
}