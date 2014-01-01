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

#include <precompiled.h>

#include <Learning.h>
#include <Errors.h>

LearningData::LearningData(int varCount)
{
	indexes.clear();
	indexes.resize(varCount);
	trainingSets.clear();
}

bool LearningData::AddSet(const vector<int>& set)
{
	bool isHomogen = true;
	for( int i = 0; i < set.size(); i++ ) {
		GetLog() << set[i] << " ";
	}
	GetLog() << endl;
	for( int i = 0; i < set.size(); i++ ) {
		PrefAssert( set[i] < indexes.size() && set[i] >= 0 );
		if( set[i] != set[0] ) {
			isHomogen = false;
		}
	}
	if( isHomogen ) {
		return false;
	}
	trainingSets.push_back(set);
	for( int i = 1; i < set.size(); i++ ) {
		indexes[set[i]].push_back(trainingSets.size() - 1);
	}
	GetLog() << "Data set was added" << endl;
	for( int i = 0; i < set.size(); i++ ) {
		GetLog() << set[i] << " ";
	}
	GetLog() << endl;
	return true;
}

static const char _ranks[] = "789TJQKA";

void LearningData::Dump()
{
	for( int i = 0; i < trainingSets.size(); i++ ) {
		for( int j = 0; j < trainingSets[i].size(); j++ ) {
			GetLog() << trainingSets[i][j] << " ";
		}
		GetLog() << endl;
		for( int j = 0; j < trainingSets[i].size(); j++ ) {
			int mask = trainingSets[i][j] & 0xff;
			PrefAssert( mask != 0 );
			int _size = (trainingSets[i][j] >> 8);
			for( int k = 0; k < 8; k++ ) {
				if( (mask & (1<<k)) != 0 ) {
					GetLog() << _ranks[k];
				}
			}
			GetLog() << "_" << _size << " " << endl;
		}
		GetLog() << endl;
	}
}

//--------------------------------------------------------------------------
//
static float CalculateDerivative(const LearningData& data, const vector<float>& v0, int index)
{
	float result = 0.0f;
	for( int i = 0; i < data.GetIndexes()[index].size(); i++ ) {
		int setIndex = data.GetIndexes()[index][i];
		if( index == data.GetTrainingSets()[setIndex][0] ) {
			result += 1.0f / v0[data.GetTrainingSets()[setIndex][0]];
		}
		float sum = 0.0f;
		for( int j = 1; j < data.GetTrainingSets()[setIndex].size(); j++ ) {
			sum += v0[data.GetTrainingSets()[setIndex][j]];
		}
		result -= 1.0f / sum;
	}
	return result;
}

static const int MAX_ITERATIONS = 50;
vector<float> Learn(const LearningData& data)
{
	vector<float> vcur(data.GetIndexes().size(), 0.5f);
	for( int i = 0; i < MAX_ITERATIONS; i++ ) {
		GetLog() << "ITERATION " << i << endl;
		vector<float> vnext(data.GetIndexes().size());
		vector<float> tmp = vcur;
		for( int j = 0; j < data.GetIndexes().size(); j++ ) {
			float min = 0.0f;
			float max = 1.0f;
			GetLog() << "Optimizing index " << j << endl;
			while( max - min > 1e-6f ) {
				GetLog() << max << " " << min << endl;
				float med = (max + min) / 2.0f;
				tmp[j] = med;
				if( CalculateDerivative(data, tmp, j) > 0.0f ) {
					min = med;
				} else {
					max = med;
				}
				tmp[j] = vcur[j];
			}
			vnext[j] = (max + min) / 2.0f;
			if( vnext[j] < 1e-6f ) {
				vnext[j] = 0.0f;
			}
		}
		vcur = vnext;
	}
	GetLog() << data.GetIndexes().size() << endl;
	GetLog() << data.GetTrainingSets().size() << endl;
	return vcur;
}

