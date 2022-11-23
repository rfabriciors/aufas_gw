cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "core" "components/esp32-wifi-manager/main" )
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(aufas_gw)