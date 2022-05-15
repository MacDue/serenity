serenity_component(
  ResourceServer
    REQUIRED
    TARGETS ResourceServer
)

compile_ipc(ResourceServer.ipc ResourceServerEndpoint.h)
compile_ipc(ResourceClient.ipc ResourceClientEndpoint.h)


set(SOURCES
    ConnectionFromClient.cpp
    main.cpp
    ResourceServerEndpoint.h
    ResourceClientEndpoint.h
)

serenity_bin(ResourceServer)
target_link_libraries(ResourceServer LibCore LibIPC LibMain)
