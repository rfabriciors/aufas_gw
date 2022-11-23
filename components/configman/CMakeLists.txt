set(REQUIRES_COMPONENTS json core carga app_update)
set(SOURCES "src/configmanager.cpp")
set(COMPONENT_ADD_INCLUDEDIRS "." "inc/" "../carga/inc/")
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    REQUIRES ${REQUIRES_COMPONENTS}
    )