cmake_minimum_required(VERSION 3.12)
project(HuePi C)

set(CMAKE_C_STANDARD 11)

add_executable(HuePi hueDriver.c hue.c huePi.h httpRequests.c cJSON.c http_parser.c)