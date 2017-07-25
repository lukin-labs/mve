file (GLOB HEADERS "*.h")
file (GLOB SOURCES "[^_]*.cc")

add_library(smvs STATIC ${SOURCES} ${HEADERS})
target_link_libraries(smvs mve)

