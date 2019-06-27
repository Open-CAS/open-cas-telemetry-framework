# The IO tracing in a C Application

First in this tutorial, generic deployment view of tracing is provided. Then the
example demonstrates how to utilize OCTF IO tracing capability in a C type
application which is the traces producer. Next the procedure of starting and
stopping tracing is shown. And finally, the culmination is showing trace parsing.   

In this tutorial:

* [Tracing overview](#overview)
* [Producing IO traces by C Application](#producing)
* [Trace management](#management)
* [Parsing traces](#parsing)

<a id="overview"></a>

## Tracing overview

TBD

<a id="producing"></a>

## Producing IO traces by C Application

All fun starts with including C IO trace plug-in.

~~~{.c}
#include <octf/c/iotrace_plugin.h>
~~~

Having that you can power your
C application by OCTF. But actually in this case you are going to use C wrapper
for IO tracer plug-in. When linking your program, don't forget about adding
OCTF library (just add -loctf to linker option).  

### Initializing IO trace plug-in

~~~{.c}
// The handle of your plug-in and its context
octf_iotrace_plugin_context_t context;

// Configuration for you IO trace plug-in
struct octf_iotrace_plugin_cnfg cnfg = {
        .id = "APP_NAME-AND_ID", // Unique ID
        .io_queue_count = 1, // Number of your IO queues
};

// Create and start IO trace plug-in
if (octf_iotrace_plugin_create(&cnfg, &plugin)) {
    // Oops, error occurred, handle it
    return -1;
}
~~~

### IO tracing

Are you ready to trace your IO workload?

~~~{.c}
// Checks if IO tracing is active
if (octf_iotrace_plugin_is_tracing_active(context)) {
    // Someone enabled IO tracing and we can trace an IO
    
    // Define the IO trace event
    struct iotrace_event ev = {};
    
    // Initialize the IO trace event header
    octf_iotrace_plugin_init_trace_header(context, &ev.hdr,
            iotrace_event_type_io, sizeof(ev));
            
    // Fill the IO trace
    ev.lba = ...;
    ev.len = ...;
    ev.operation = ...;
    ...
    
    // Push the IO trace into the IO tracing queue 0
    octf_iotrace_plugin_push_trace(context, 0, &ev, sizeof(ev));
    
    // You all set, the IO trace captured
}
~~~

### De-Initializing IO trace plug-in

Of course when exiting application it would be great to cleanup.

~~~{.c}
octf_iotrace_plugin_destroy(&context);   
~~~

<a id="management"></a>

## Trace management

TBD

<a id="parsing"></a>

## Parsing traces 

TBD
