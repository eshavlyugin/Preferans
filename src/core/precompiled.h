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

// Configure
#include <PreferansConfig.h>

// std includes
#include <vector>
#include <string>
#include <queue>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <memory.h>
#include <stdexcept>
#include <typeinfo>
#include <math.h>
#include <algorithm>

// boost includes
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/scoped_ptr.hpp>

using namespace std;

using boost::array;
using boost::multi_array;
using boost::shared_ptr;
using boost::scoped_ptr;
using boost::unordered_map;

// Local includes
//

#include "PrefTypes.h"

using namespace Preference;

#include "Random.h"
#include "Layout.h"
#include "Frac.h"
#include "Log.h"
