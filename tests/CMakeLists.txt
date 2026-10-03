# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
include(Catch)
add_library(cs_test_support STATIC support/Analysis.cpp)
target_include_directories(cs_test_support PUBLIC support)

add_executable(cs_support_selftest support_selftest/SelfTest.cpp)
target_link_libraries(cs_support_selftest PRIVATE cs_test_support Catch2::Catch2WithMain)
catch_discover_tests(cs_support_selftest ADD_TAGS_AS_LABELS)

add_executable(cs_tests unit/test_instruments.cpp unit/test_dsp_blocks.cpp unit/test_tuning.cpp
                        acoustic/test_string.cpp acoustic/test_engine.cpp
                        acoustic/test_premortem.cpp)
find_package(Threads REQUIRED)
target_link_libraries(cs_tests PRIVATE Threads::Threads)
target_link_libraries(cs_tests PRIVATE cs_core cs_test_support Catch2::Catch2WithMain)
catch_discover_tests(cs_tests ADD_TAGS_AS_LABELS DISCOVERY_MODE PRE_TEST)

add_executable(cs_rt_tests rt/test_rt_safety.cpp)
target_link_libraries(cs_rt_tests PRIVATE cs_core Catch2::Catch2WithMain)
catch_discover_tests(cs_rt_tests ADD_TAGS_AS_LABELS DISCOVERY_MODE PRE_TEST)

add_executable(cs_perf_tests perf/test_perf.cpp perf/test_perf_tail.cpp)
target_link_libraries(cs_perf_tests PRIVATE cs_core Catch2::Catch2WithMain)
catch_discover_tests(cs_perf_tests ADD_TAGS_AS_LABELS DISCOVERY_MODE PRE_TEST)

if(TARGET cs_plugin_core)
  add_executable(cs_plugin_tests plugin/test_state.cpp)
  target_link_libraries(cs_plugin_tests PRIVATE cs_plugin_core cs_core Catch2::Catch2WithMain)
  catch_discover_tests(cs_plugin_tests ADD_TAGS_AS_LABELS DISCOVERY_MODE PRE_TEST)
endif()

add_test(NAME T-113-licence-notice COMMAND ${CMAKE_COMMAND} -E env python3 tools/check_notice.py WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties(T-113-licence-notice PROPERTIES LABELS "repo")

if(CS_FUZZ AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  foreach(t scala state)
    add_executable(fuzz_${t} fuzz/fuzz_${t}.cpp)
    target_compile_options(fuzz_${t} PRIVATE -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined -g -O1)
    target_link_options(fuzz_${t} PRIVATE -fsanitize=fuzzer,address,undefined)
  endforeach()
  target_link_libraries(fuzz_scala PRIVATE cs_core)
  if(TARGET cs_plugin_core)
    target_link_libraries(fuzz_state PRIVATE cs_plugin_core cs_core)
  endif()
endif()
