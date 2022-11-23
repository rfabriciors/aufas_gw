set(SOURCES "src/main.cpp" "src/funcoes.cpp"
    )

set(COMPONENT_ADD_INCLUDEDIRS "." "inc/" "../components/esp32-wifi-manager/main" "../components/mqttauto/inc" "../components/carga/inc" "../components/configman/inc" )

set(REQUIRES_COMPONENTS log nvs_flash mdns wpa_supplicant lwip esp_http_server json mqtt configman bootloader_support)
set(EMBED "../components/esp32-wifi-manager/main/style.css"
    "../components/esp32-wifi-manager/main/code.js"
    "../components/esp32-wifi-manager/main/index.html")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
                       REQUIRES ${REQUIRES_COMPONENTS}
                        EMBED_FILES ${EMBED}
                       )