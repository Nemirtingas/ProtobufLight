cmake_minimum_required(VERSION 3.15)
project(ProtobufLightTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf CONFIG REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_BINARY_DIR}
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS protobuf.proto)

add_executable(ProtobufLightTests
    main.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_link_libraries(ProtobufLightTests
    PRIVATE ProtobufLight protobuf::libprotobuf-lite
)

enable_testing()
add_test(NAME ProtobufLightTests COMMAND ProtobufLightTests)
