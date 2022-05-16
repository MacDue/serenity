set(SOURCES
    PathResolverClient.cpp
)

set(GENERATED_SOURCES
    ../../Services/ResourceServer/ResourceClientEndpoint.h
    ../../Services/ResourceServer/ResourceServerEndpoint.h
)

serenity_lib(LibResource resource)
target_link_libraries(LibResource LibCore LibIPC)
