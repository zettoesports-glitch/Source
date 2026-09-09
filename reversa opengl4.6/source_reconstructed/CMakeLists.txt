cmake_minimum_required(VERSION 3.20)
project(MainExeReverseGL46 LANGUAGES CXX)

set(REVERSE_GL46_COMMON
    core/GL46Capabilities.cpp
    core/GLCompatFixedState.cpp
    core/RendererModel.cpp
    core/Renderer2DGL46.cpp
    core/RendererTerrainGL46.cpp
    core/RendererWaterGL46.cpp
    core/RendererSkyGL46.cpp
    core/RendererAtmosphereGL46.cpp
    core/RendererClothGL46.cpp)

add_library(MainExeReverseGL46 STATIC ${REVERSE_GL46_COMMON})
target_include_directories(MainExeReverseGL46 PUBLIC include)
target_compile_features(MainExeReverseGL46 PUBLIC cxx_std_17)

if(WIN32)
  target_sources(MainExeReverseGL46 PRIVATE
      core/RendererSelection.cpp
      core/RenderDevice3D.cpp
      core/Winmain_RendererBootstrap.cpp)
  target_link_libraries(MainExeReverseGL46 PUBLIC opengl32 advapi32 glew32)
endif()
