# The Auto CLI

The OCTF introduces a functionality called "Auto CLI". Generally this capability
on the basis of google protocol buffer definition builds automatically a CLI
tool. In this example we are going to guide you to have own CLI utilizing OCTF.

In this tutorial:

* [Protocol Buffer Basis](#protocol_buffer_basis)
* [Specifying CLI Schema](#specifying_cli_schema)
* [Method Implementation](#method_implementation)
* [Main Function](#main_function)
* [Resources](#resources)

<a id="protocol_buffer_basis"></a>

## Google Protocol Buffer Basis

What are protocol buffers? According to [Google Protocol Buffer Basis](https://developers.google.com/protocol-buffers/):
"Protocol buffers are Google's language-neutral, platform-neutral, extensible mechanism for serializing structured data - think XML, but smaller, faster, and simpler. You define how you want your data to be structured once, then you can use special generated source code to easily write and read your structured data to and from a variety of data streams and using a variety of languages."

Assuming you defined a protocol buffer message:

~~~{.proto}
message Request {
    string hello = 1;
}
~~~

Using protocol buffer CPP compiler you get the following auto generator code:  

~~~{.cpp}
class Request : public ::google::protobuf::Message {
 public:
  Request();
  virtual ~Request();

  ...
  ...
  ...

  // accessors -------------------------------------------------------

  // string hello = 1;
  void clear_hello();
  const ::std::string& hello() const;
  void set_hello(const ::std::string& value);
};
~~~

In addition the protocol buffer allows to define a service - know as a
**interface** in OCTF world. The following **InterfaceHelloWorld** interface
defines a method **HelloWorld**. The method accept the **Request** message,
the result of this method is the message **Response**.   

~~~{.proto}
message Request {
    string hello = 1;
}

message Response {
    string helloResponse = 1;
}

service InterfaceHelloWorld {
    rpc HelloWorld(Request) returns (Response);
}
~~~

we should expect auto generated code:

~~~{.cpp}
/*
 * Message which will be used as a method input
 */
class Request : public ::google::protobuf::Message {
 public:
  Request();
  virtual ~Request();

  ...
  ...
  ...

  // accessors -------------------------------------------------------

  // string hello = 1;
  void clear_hello();
  const ::std::string& hello() const;
  void set_hello(const ::std::string& value);
};

/*
 * Message which will be used as a method output
 */
class Response : public ::google::protobuf::Message {
 public:
  Response();
  virtual ~Response();

  ...
  ...
  ...

  // accessors -------------------------------------------------------

  // string helloResponse = 1;
  void clear_helloresponse();
  const ::std::string& helloresponse() const;
  void set_helloresponse(const ::std::string& value);
};


class InterfaceHelloWorld : public ::google::protobuf::Service {
 protected:
  inline InterfaceHelloWorld() {};

 public:
  virtual ~InterfaceHelloWorld();

  ...
  ...
  ...

  /*
   * Methods to override
   */
  virtual void HelloWorld(::google::protobuf::RpcController* controller,
                       const ::Request* request,
                       ::Response* response,
                       ::google::protobuf::Closure* done);
};
~~~

Then you can use above source stubs to implements your own interface as it will
be presented in the further part of this tutorial. 

<a id="specifying_cli_schema"></a>

## Specifying CLI Schema

The OCTF uses built-in protocol buffers descriptions and reflection to form CLI
commands and their parameters. However we have to get CLI meta information about:
- CLI commands switches (long key or short) and their help description,
- CLI parameters switches (long key or short) and their help description,
- CLI parameters validation info (for instance the range of a number parameter)
- etc...

Thus the OCTF defines [CLI options](https://github.com/Open-CAS/open-cas-telemetry-framework/blob/master/source/octf/proto/opts.proto) which allows to provide above specification.

Let's take a look what the full specification of our CLI should be:

~~~{.proto}
syntax = "proto3";
option cc_generic_services = true;
import "opts.proto";

message Request {
    string hello = 1 [
        // this parameter is required to be set by user in terminal command line
        (octf.proto.opts_param).cli_required = true,

        // this parameter can be set by using -L <VALUE> option
        (octf.proto.opts_param).cli_short_key = "L",

        // this parameter can be set by using --hello <VALUE> option
        (octf.proto.opts_param).cli_long_key = "hello",

        // this is help for this parameter
        (octf.proto.opts_param).cli_desc = "Say hello"
    ];
}

message Response {
    string helloResponse = 1 [
        // defines name of parameter displayed in user's terminal
        (octf.proto.opts_param).cli_desc = "Hello Response"
    ];
}

service InterfaceHelloWorld {
    // Enables this interface in CLI
    option (octf.proto.opts_interface).cli = true;

    // Specifies version of interface, if missed, version is set to 0 by default
    option (octf.proto.opts_interface).version = 1;

    // Example method
    rpc HelloWorld(Request) returns (Response) {
        // Now this command can be executed by auto CLI
        option (octf.proto.opts_command).cli = true;

        // this command can be invoked when typing '-W'
        option (octf.proto.opts_command).cli_short_key = "W";

        // this command can be invoked when typing "--hello-world"
        option (octf.proto.opts_command).cli_long_key = "hello-world";

        // description of command displayed when printing help
        option (octf.proto.opts_command).cli_desc =
            "This is hello world command";
    }
}
~~~

<a id="method_implementation"></a>

## Method Implementation

Now we can implement methods of our interface. It's easy task. Just create
a class which inherits by **InterfaceHelloWorld**, overrides your methods and
provide implementation. In our case we have the following class definition:

~~~{.cpp}
#include "InterfaceHelloWorld.pb.h"

class InterfaceHelloWorldImpl : public InterfaceHelloWorld {
public:
    InterfaceHelloWorldImpl() = default;
    virtual ~InterfaceHelloWorldImpl() = default;

    virtual void HelloWorld(::google::protobuf::RpcController *controller,
                            const ::Request *request,
                            ::Response *response,
                            ::google::protobuf::Closure *done) override;
};
~~~

and implementation:

~~~{.cpp}
#include "InterfaceHelloWorldImpl.h"

#include <string>
#include <octf/octf.h>

void InterfaceHelloWorldImpl::HelloWorld(
        ::google::protobuf::RpcController *controller,
        const ::Request *request,
        ::Response *response,
        ::google::protobuf::Closure *done) {
    if ("" == request->hello()) {
        // Terminate this method with the error
        controller->SetFailed("Error. You have to say something");
        done->Run();
        return;
    }

    if ("magic" == request->hello()) {
        // Or just throw exception in order to report command execution error
        throw octf::Exception("I don't like magic");
    }

    // Prepare response
    std::string value = "Hi, this is CLI. You typed: ";
    value += request->hello();

    // Set response
    response->set_helloresponse(value);

    // End method
    done->Run();
}
~~~

<a id="main_function"></a>

## Main Function

The next step is implementation of main function:
- Create a CLI executor
- Set name and version of your CLI
- Adds interface to the executor
- **Run executor which consumes user arguments**

~~~{.cpp}
#include <memory>
#include <string>
#include <octf/octf.h>
#include "InterfaceHelloWorldImpl.h"

int main(int argc, char *argv[]) {
    // Create executor and local command set
    octf::cli::Executor ex;

    // Configure CLI program
    auto &properties = ex.getCliProperties();
    properties.setName("example-cli");
    properties.setVersion("1.0");

    // Create interface
    octf::InterfaceShRef iHelloWord =
            std::make_shared<InterfaceHelloWorldImpl>();

    // Add interface to executor
    ex.addModules(iHelloWord);

    // Execute command
    return ex.execute(argc, argv);
}
~~~

## Help

Once the program is ready we can get the program's help:

~~~{.sh}
./auto-cli --help
Usage: example-cli command [options...]

Available commands: 
     -W    --hello-world                         This is hello world command
     -H    --help                                Print help
     -V    --version                             Print version
~~~

and the hello-world command's help:

~~~{.sh}
Usage: ./auto-cli --hello-world --hello <VALUE> 

This is hello world command

Options that are valid with {-W | --hello-world}  
     -L    --hello <VALUE>                       Say hello
~~~

## Using your CLI

Invoking our method is icing on the cake 

~~~{.sh}
./auto-cli --hello-world --hello "Hi OCTF"
{
 "helloResponse": "Hi, this is CLI. You typed: Hi OCTF"
}

# An invalid call
./auto-cli --hello-world --hello ""
{
 "timestamp": "2019-07-22 19:38.929",
 "system": "example-cli",
 "severity": "Error",
 "trace": "Error. You have to say something"
}
~~~

**Good luck with your own CLI!!!**

## Resources
- [This example sources](https://github.com/Open-CAS/open-cas-telemetry-framework/tree/master/source/examples/auto_cli)
- [Standalone Linux IO Tracer CLI](https://github.com/Open-CAS/standalone-linux-io-tracer/blob/master/source/userspace/main.cpp)
