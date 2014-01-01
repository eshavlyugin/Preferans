find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(RSVG librsvg-2.0)

include_directories( ${GTKMM_INCLUDE_DIRS} ${RSVG_INCLUDE_DIRS} )
add_executable (Preferans precompiled.h BiddingDialog.cpp
BulletWidget.cpp
CardWidget.cpp
ConnectionManager.cpp
DealScoreWidget.cpp
GameController.cpp
ImagesStorage.cpp
main.cpp
PlayerBidWidget.cpp
PrefSlots.cpp
PrefView.cpp
PrefWindow.cpp)
target_link_libraries( Preferans ${GTKMM_LIBRARIES} ${RSVG_LIBRARIES} PrefCore )
install (TARGETS Preferans DESTINATION bin)
