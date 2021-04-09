cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_definitions(CONFIG_EXAMPLE_PROV_TRANSPORT_SOFTAP)
project(esp32genericrtos)
