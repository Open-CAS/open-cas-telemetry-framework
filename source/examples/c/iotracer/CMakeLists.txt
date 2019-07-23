add_executable(c-example-iotracer "")
target_link_libraries(c-example-iotracer octf)
target_sources(c-example-iotracer PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/main.c
)
