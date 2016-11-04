cmake_minimum_required (VERSION 2.8)

set(CMAKE_CXX_STANDARD 14)

project(Preferans)

find_package (Threads)
find_package(GTest)
find_package(Boost 1.40 COMPONENTS program_options python REQUIRED )
find_package(PythonLibs 2.7 REQUIRED)

ADD_LIBRARY(PrefLib
  common.cpp
  features.cpp
  gamemgr.cpp
  generate.cpp
  layout_sample.cpp
  monte_carlo.cpp
  player.cpp
  playout.cpp
  train_model.cpp
)

set_property(TARGET PrefLib PROPERTY POSITION_INDEPENDENT_CODE TRUE)

include_directories(${PYTHON_INCLUDE_DIRS})

add_executable(UnitTest common_ut.cpp layout_sample_ut.cpp players_ut.cpp run_ut.cpp)
add_executable(TestGame 
  game.cpp 
  pymodels.cpp
)

target_link_libraries(TestGame PrefLib ${CMAKE_THREAD_LIBS_INIT} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
target_link_libraries(UnitTest PrefLib ${GTEST_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(pywrap)
