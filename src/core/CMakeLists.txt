add_library(PrefCore AIPrefPlayer.cpp
Bullet.cpp
ContractGamePolicy.cpp
DealState.cpp
EngineData.cpp
EngineSettings.cpp
Errors.cpp
GameSettings.cpp
Learning.cpp
LocalPrefServer.cpp
MiserePolicy.cpp
PassoutPolicy.cpp
PrefEngineImpl.cpp
Preference.cpp
PrefGameModelImpl.cpp
PrefTools.cpp
RandomLayoutsGenerator.cpp
RemotePrefPlayer.cpp
RemotePrefServer.cpp)

add_executable(PrefDatabaseConverter DBConverter.cpp DatabaseParser.cpp main.cpp)
target_link_libraries(PrefDatabaseConverter PrefCore)

