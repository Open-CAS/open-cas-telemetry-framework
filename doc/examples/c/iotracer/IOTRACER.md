# The IO tracing in a C Application

First in this tutorial, generic deployment view of tracing is provided. Then the
example demonstrates how to utilize OCTF IO tracing capability in a C type
application which is the traces producer. Next the procedure of starting and
stopping tracing is shown. And finally, at the end we present how to parse the
trace.   

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

All the fun starts with including the C IO trace plug-in.

~~~{.c}
#include <octf/c/iotrace_plugin.h>
~~~

Having that you can power your C application by OCTF. But actually in this case
you are going to use C wrapper for IO tracer plug-in. When linking your program,
don't forget about adding OCTF library (just add -loctf to linker options).  

### Initializing IO trace plug-in

~~~{.c}
// The handle of your plug-in and its context
octf_iotrace_plugin_context_t context;

// Configuration for your IO trace plug-in
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

### Starting traces
Once the plug-in has been created, the user can start tracing or stop it using
a CLI. One option would be calling iotrace CLI from 
[Standalone Linux IO Tracer](https://github.com/Open-CAS/standalone-linux-io-tracer).

Assuming the plug-in id is: "c-iotrace-example", call iotrace help:

~~~{.sh}
iotrace --help
Usage: iotrace [plugin] command [options...]

Available modules: 
           --c-iotrace-example
~~~

Then you can get help for your plugin: 

~~~{.sh}
iotrace --c-iotrace-example --help
Usage: iotrace --c-iotrace-example command [options...]

Available commands: 
           --get-trace-status                    This command will return the status of an ongoing telemetry collection
     -H    --help                                Print help
           --start-trace                         This command will begin collecting telemetry data from the drive
           --stop-trace                          This command will preemptively end collecting of telemetry data from the drive
~~~

Now you can start tracing by invoking:

~~~{.sh}
iotrace --c-iotrace-example --start-trace
~~~

Print help of start tracing command to get more tracing options. 

~~~{.sh}
iotrace --c-iotrace-example --start-trace --help 
Usage: iotrace --c-iotrace-example --start-trace [options...]

This command will begin collecting telemetry data from the drive

Options that are valid with --start-trace  
     -b    --buffer <1-1024>                     Size of the internal trace buffer (in MiB) (default: 100)
     -d    --duration <10-3600>                  Max trace duration time (in seconds) (default: 3600)
     -s    --size <1-4096>                       Max size of trace file (in MiB) (default: 4096)
~~~


> **NOTE:** Second possibility is building own CLI tool which involves
[InterfaceTraceCreatingImpl](https://github.com/Open-CAS/open-cas-telemetry-framework/blob/master/source/octf/interface/InterfaceTraceCreatingImpl.h)

### IO tracing

Are you ready to trace your IO workload?

~~~{.c}
// Checks if IO tracing is started
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
    
    // You're all set, the IO trace captured
}
~~~

### De-Initializing IO trace plug-in

Of course when exiting application it would be great to clean up.

~~~{.c}
octf_iotrace_plugin_destroy(&context);   
~~~

<a id="management"></a>

## Trace management

TBD

<a id="parsing"></a>

## Parsing traces 

TBD
