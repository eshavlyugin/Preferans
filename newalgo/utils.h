/*
 * utils.h
 *
 *  Created on: 3 июля 2016 г.
 *      Author: zhenyok
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <bits/stdc++.h>

using namespace std;

namespace utils {

inline vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

template<typename T>
vector<T> splitString(const string& str) {
	istringstream sstr(str);
	vector<T> result;
	while (sstr.good()) {
		result.push_back(T());
		sstr >> result.back();
	}
	return result;
}

} // namespace utils

#endif /* UTILS_H_ */
