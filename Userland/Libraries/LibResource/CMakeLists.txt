set(SOURCES
    PathResolverClient.cpp
)

serenity_lib(LibResource resource)
target_link_libraries(LibResource LibCore LibIPC)
