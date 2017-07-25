file (GLOB HEADERS "*.h")
file (GLOB SOURCES "*.cc")

add_executable(smvs_app ${SOURCES} ${HEADERS})
target_link_libraries(smvs_app mve smvs mve_sfm mve_util)
install(TARGETS smvs_app RUNTIME DESTINATION .)
