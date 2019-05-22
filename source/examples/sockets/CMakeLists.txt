add_executable(sockets "")
# Specify libraries to link - specifies dependencies
# and allows valid concurrent build
target_link_libraries(sockets PUBLIC octf)
target_link_libraries(sockets PUBLIC ${PROTOBUF_LIBRARIES})
target_sources(sockets
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/socketsExampleMain.cpp
)
