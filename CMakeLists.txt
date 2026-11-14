cmake_minimum_required(VERSION 3.22)
project(Axell76 VERSION 3.2.0)
add_compile_definitions(AXELL_VERSION_STRING="${PROJECT_VERSION}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# Windows (MSVC): static runtime (no VC++ redistributable needed), M_PI, UTF-8 sources; GCC-style flags elsewhere only
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_definitions(_USE_MATH_DEFINES)
  add_compile_options(/utf-8)
  # the test programs hold several ~200 KB engines as locals: 16 MB stack like Linux's 8 MB, not Windows' 1 MB (executables only;
  # the VST3 is a DLL and the host creates the plugin on the heap)
  string(APPEND CMAKE_EXE_LINKER_FLAGS " /STACK:16777216")
  set(A76_TEST_FLAGS /W3)
  set(A76_FORMATS VST3 Standalone)
else()
  set(A76_TEST_FLAGS -Wall -Wextra -Wshadow $<$<CONFIG:Release>:-O3>)
  set(A76_FORMATS VST3 LV2 Standalone)
endif()

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.15
  GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

set(AXELL_CORE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/axell-core" CACHE PATH "Axell core")
if(NOT EXISTS "${AXELL_CORE_DIR}/CMakeLists.txt")
  set(AXELL_CORE_DIR "${CMAKE_CURRENT_LIST_DIR}/../axell-core" CACHE PATH "Axell core" FORCE)
endif()
set(AXELL_CORE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${AXELL_CORE_DIR} axell-core)

juce_add_plugin(Axell76
  COMPANY_NAME              "Axell Yobag"
  COMPANY_EMAIL             "axellpetre@protonmail.com"
  BUNDLE_ID                 com.axellyobag.axell76
  PLUGIN_MANUFACTURER_CODE  Axyb
  PLUGIN_CODE               Ax76
  FORMATS                   ${A76_FORMATS}
  LV2URI                    "https://github.com/zmardanian/axell-76"
  PRODUCT_NAME              "Axell 76"
  IS_SYNTH                  FALSE
  NEEDS_MIDI_INPUT          FALSE
  NEEDS_MIDI_OUTPUT         FALSE
  IS_MIDI_EFFECT            FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD   FALSE
  VST3_CATEGORIES           Fx Dynamics)

juce_add_binary_data(A76Fonts HEADER_NAME A76Fonts.h NAMESPACE A76Fonts SOURCES
  Resources/fonts/GreatVibes-Regular.ttf)
set_target_properties(A76Fonts PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(A76_DEFS JUCE_WEB_BROWSER=0 AXELL_THEME_HUE=215 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_sources(Axell76 PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(Axell76 PUBLIC ${A76_DEFS})
axell_core_add_gui(Axell76)
target_link_libraries(Axell76 PRIVATE A76Fonts juce::juce_audio_utils
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# JUCE-free DSP tests + an offline renderer (the engine exactly as it ships)
enable_testing()
add_executable(A76DspTests tests/DspTests.cpp)
target_include_directories(A76DspTests PRIVATE Source)
target_link_libraries(A76DspTests PRIVATE axell_core_dsp)
target_compile_options(A76DspTests PRIVATE ${A76_TEST_FLAGS})
add_test(NAME dsp_suite COMMAND A76DspTests)
add_executable(a76_render tools/a76_render.cpp)
target_include_directories(a76_render PRIVATE Source)
target_link_libraries(a76_render PRIVATE axell_core_dsp)
if(NOT MSVC)
  target_compile_options(a76_render PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

# headless GUI snapshot (JUCE)
juce_add_console_app(A76GuiSnapshot PRODUCT_NAME "A76GuiSnapshot")
target_compile_definitions(A76GuiSnapshot PRIVATE ${A76_DEFS} JUCE_MODAL_LOOPS_PERMITTED=1 JucePlugin_Name="Axell 76")
target_sources(A76GuiSnapshot PRIVATE tests/GuiSnapshot.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_link_libraries(A76GuiSnapshot PRIVATE A76Fonts juce::juce_audio_utils juce::juce_recommended_config_flags)
axell_core_add_gui(A76GuiSnapshot)

# plugin-level tests (JUCE): state round trip, reload render identity, bypass null
juce_add_console_app(A76PluginTests PRODUCT_NAME "A76PluginTests")
target_compile_definitions(A76PluginTests PRIVATE ${A76_DEFS} JucePlugin_Name="Axell 76")
target_sources(A76PluginTests PRIVATE tests/PluginTests.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_link_libraries(A76PluginTests PRIVATE A76Fonts juce::juce_audio_utils juce::juce_recommended_config_flags)
axell_core_add_gui(A76PluginTests)
add_test(NAME plugin_suite COMMAND A76PluginTests)
