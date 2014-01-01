/****************************************************************************
 Preferans: implementation of card-tricking game Preferans (or Preference).
 ****************************************************************************
 Copyright (c) 2010-2011  Eugene Shavlyugin <eshavlyugin@gmail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ****************************************************************************/

#ifndef _ENGINE_SETTINGS_H_
#define _ENGINE_SETTINGS_H_

namespace Preference {

struct EngineSettings {
	EngineSettings();

	// Number of passout simulations per layout
	int NumOfPassoutSimulations;
	// Number of passout layouts to solve
	int NumOfPassoutLayouts;
	
	// Misere options
	// Number of layouts to test before reject misere
	int MaxMisereFailures;
	// Number of layouts to test to accept misere
	int MinMisereWins;
	// Number of contract playouts
	int ContractGameChecks;
	// Number of random layouts to play in contract game or misere
	int NumOfSamplesPerMove;

	void Load();
	void Save();
};

inline EngineSettings::EngineSettings() :
	NumOfPassoutSimulations(0),
	NumOfPassoutLayouts(0),
	MaxMisereFailures(0),
	MinMisereWins(0),
	ContractGameChecks(0),
	NumOfSamplesPerMove(0)
{
}

} // namespace Preference

#endif // _ENGINE_SETTINGS_H_
