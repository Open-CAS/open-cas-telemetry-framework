add_subdirectory(internal)
target_sources(octf
PRIVATE
   ${CMAKE_CURRENT_LIST_DIR}/ITraceSerializer.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceCliImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceServiceImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceServiceImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceManagementImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/ITraceExecutor.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceId.h
   ${CMAKE_CURRENT_LIST_DIR}/TraceProducerLocal.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceCliImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceId.cpp
   ${CMAKE_CURRENT_LIST_DIR}/ITraceConverter.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceParsingImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceParsingImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceCreatingImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceCreatingImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceIdentificationImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/IRingTraceProducer.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceManagementImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceIdentificationImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceConfigurationImpl.h
   ${CMAKE_CURRENT_LIST_DIR}/InterfaceConfigurationImpl.cpp
   ${CMAKE_CURRENT_LIST_DIR}/TraceProducerLocal.cpp
   ${CMAKE_CURRENT_LIST_DIR}/TraceManager.h
   ${CMAKE_CURRENT_LIST_DIR}/TraceManager.cpp
   ${CMAKE_CURRENT_LIST_DIR}/TraceConverter.h
   ${CMAKE_CURRENT_LIST_DIR}/TraceConverter.cpp
   ${CMAKE_CURRENT_LIST_DIR}/ITraceExtensionBuilder.h
   
)
