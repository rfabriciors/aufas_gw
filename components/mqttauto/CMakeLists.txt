set(SOURCES "./src/mqtt.cpp")
set(COMPONENT_ADD_INCLUDEDIRS "." "inc/" "../../core/inc/" "../configman/inc/")
set(REQUIRES_COMPONENTS mqtt json core configman)
idf_component_register(SRCS ${SOURCES}
                        INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
                        REQUIRES ${REQUIRES_COMPONENTS}
                        )