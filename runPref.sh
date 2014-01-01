#!/bin/sh
if [ -r src/gui/gtkmm/Preferans ] 
then
	#valgrind --tool=callgrind src/gui/gtkmm/Preferans
	src/gui/gtkmm/Preferans
else
	echo "You need to compile Preferans first"
fi
