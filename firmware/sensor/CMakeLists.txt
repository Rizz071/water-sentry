cmake_minimum_required(VERSION 3.16.0)

# Добавляем путь к нашей общей папке в список директорий с компонентами
#list(APPEND EXTRA_COMPONENT_DIRS "../shared_libs")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(active_sender)
