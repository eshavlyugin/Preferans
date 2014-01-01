# Precompiled header
BUILT_SOURCES = precompiled.h.gch

PREF_INCLUDES = -I../../../include 

precompiled.h.gch: precompiled.h ../../../include/*
	$(CXX) $(PREF_INCLUDES) `pkg-config --cflags librsvg-2.0` `pkg-config --cflags gtkmm-2.4` $(DEFAULT_INCLUDES) -Wall $(CXXFLAGS) $(AM_CXXFLAGS) -DDEBUG_MODE -g $<

clean-local:
	rm -f precompiled.h.gch

# Main program
bin_PROGRAMS = preferans

preferans_SOURCES = BiddingDialog.cpp \
	BulletWidget.cpp \
	CardWidget.cpp \
	ConnectionManager.cpp \
	DealScoreWidget.cpp \
	GameController.cpp \
	ImagesStorage.cpp \
	PlayerBidWidget.cpp \
	PrefSlots.cpp \
	PrefView.cpp \
	PrefWindow.cpp \
	main.cpp

preferans_CXXFLAGS = $(PREF_INCLUDES) $(DEFAULT_INCLUDES) `pkg-config --cflags librsvg-2.0` `pkg-config --cflags gtkmm-2.4`
preferans_LDADD = ../../core/libPref.a
preferans_LDFLAGS = `pkg-config --libs librsvg-2.0` `pkg-config --libs gtkmm-2.4`
