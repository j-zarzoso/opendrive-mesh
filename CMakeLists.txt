cmake_minimum_required(VERSION 3.14)
project(opendrive-mesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern)

add_executable(opendrive-mesh
    src/main.cpp
    src/parser/OpenDRIVEParser.cpp
    src/geometry/Math.cpp
    src/geometry/RoadGeometry.cpp
    src/geometry/LaneMesh.cpp
    src/mesh/MeshGenerator.cpp
    src/mesh/ObjectMesh.cpp
    src/mesh/SignalMesh.cpp
    src/mesh/JunctionMesh.cpp
    src/export/USDExporter.cpp
    src/export/OBJExporter.cpp
    src/types/MeshTypes.cpp
)

target_include_directories(opendrive-mesh PRIVATE
    src
    extern/pugixml/include
)

target_link_libraries(opendrive-mesh pugixml)

if(MSVC)
    add_compile_options(/W4)
    add_custom_command(TARGET opendrive-mesh POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/install/bin"
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:opendrive-mesh>"
            "${PROJECT_SOURCE_DIR}/install/bin/"
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_custom_command(TARGET opendrive-mesh POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/install/bin"
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:opendrive-mesh>"
            "${PROJECT_SOURCE_DIR}/install/bin/"
    )
endif()

enable_testing()

add_executable(opendrive-test
    tests/test.cpp
    tests/test_main.cpp
    src/parser/OpenDRIVEParser.cpp
    src/geometry/Math.cpp
    src/geometry/RoadGeometry.cpp
    src/geometry/LaneMesh.cpp
    src/mesh/MeshGenerator.cpp
    src/mesh/ObjectMesh.cpp
    src/mesh/SignalMesh.cpp
    src/mesh/JunctionMesh.cpp
    src/export/USDExporter.cpp
    src/export/OBJExporter.cpp
    src/types/MeshTypes.cpp
)

target_include_directories(opendrive-test PRIVATE
    src
    tests
    extern/pugixml/include
)

target_link_libraries(opendrive-test pugixml)

add_test(NAME opendrive-test COMMAND opendrive-test)
