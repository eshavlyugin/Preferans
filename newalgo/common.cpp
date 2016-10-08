#include "common.h"

void ProcessAssert(const char* PREF_ASSERT, const char* file, long line) {
	cerr << "failed PREF_ASSERT " << PREF_ASSERT << " in " << file << " at " << line
			<< ".\n";
	abort();
}

AssertHandler* globalAssertHandler = &ProcessAssert;
