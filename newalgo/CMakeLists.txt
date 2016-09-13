cmake_minimum_required (VERSION 2.8)

set(CMAKE_CXX_STANDARD 14)

project (Preferans)

find_package (Threads)
find_package(GTest)
find_package(Boost 1.40 COMPONENTS program_options REQUIRED )

ADD_LIBRARY(PrefLib
  features.cpp
  gamemgr.cpp
  generate.cpp
  layout_sample.cpp
  monte_carlo.cpp
  preparedata.cpp
  player.cpp
  playout.cpp
  train_model.cpp
)

add_executable(UnitTest common_ut.cpp layout_sample_ut.cpp model_predictor_ut.cpp players_ut.cpp run_ut.cpp)
add_executable(TestGame game.cpp)
add_executable(TrainModel train.cpp)
target_link_libraries(TestGame PrefLib ${CMAKE_THREAD_LIBS_INIT} ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_link_libraries(UnitTest PrefLib ${GTEST_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(TrainModel PrefLib ${CMAKE_THREAD_LIBS_INIT} ${Boost_PROGRAM_OPTIONS_LIBRARY})
