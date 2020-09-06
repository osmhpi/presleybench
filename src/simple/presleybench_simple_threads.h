#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER presleybench_simple_threads

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "simple/presleybench_simple_threads.h"

#if !defined(_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TP_H

#include <lttng/tracepoint.h>

/*
 * Use TRACEPOINT_EVENT(), TRACEPOINT_EVENT_CLASS(),
 * TRACEPOINT_EVENT_INSTANCE(), and TRACEPOINT_LOGLEVEL() here.
 */

#endif /* _TP_H */

#include <lttng/tracepoint-event.h>

TRACEPOINT_EVENT(
    /* Tracepoint provider name */
    presleybench_simple_threads,

    /* Tracepoint name */
    execute_task,

    /* Input arguments */
    TP_ARGS(
        int, thread_id,
        int, node_id
    ),

    /* Output event fields */
    TP_FIELDS(
        ctf_integer(int, thread_id, thread_id)
        ctf_integer(int, node_id, node_id)
    )
)