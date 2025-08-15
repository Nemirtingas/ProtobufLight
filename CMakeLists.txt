cmake_minimum_required(VERSION 3.15)
project(ProtobufLight VERSION 1.0.0 LANGUAGES CXX)

add_library(ProtobufLight INTERFACE)
target_include_directories(ProtobufLight INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(ProtobufLight INTERFACE cxx_std_17)

##################
## Install rules
install(TARGETS ProtobufLight EXPORT ProtobufLightTargets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Export targets
install(DIRECTORY include/ DESTINATION ProtobufLight/)

install(EXPORT ProtobufLightTargets
    FILE ProtobufLightTargets.cmake
    NAMESPACE ProtobufLight::
    DESTINATION lib/cmake/ProtobufLight
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/ProtobufLightConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ProtobufLightConfig.cmake
    INSTALL_DESTINATION lib/cmake/ProtobufLight
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ProtobufLightConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ProtobufLight
)

option(PROTOBUF_LIGHT_ENABLE_TESTS "Build unit tests" OFF)
if (PROTOBUF_LIGHT_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()