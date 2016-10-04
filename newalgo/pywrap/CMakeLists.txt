find_package(Boost 1.40 COMPONENTS python REQUIRED )
find_package(PythonLibs 2.7 REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})

ADD_LIBRARY(Pref_pywrap SHARED
  pywrap.cpp
)

target_link_libraries(Pref_pywrap ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} PrefLib)

set_target_properties(Pref_pywrap PROPERTIES PREFIX "")