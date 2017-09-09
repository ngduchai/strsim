#include "code.h"
#include <iostream>

using namespace std;
using namespace strsim;

#define NUM_BLOCKS 20
#define NUM_TEST 10000

int main() {
	unsigned int * degrees = new unsigned int [NUM_BLOCKS];

	cout << "Test Soliton Distribution Generator" << endl;
	soliton_generator sg(NUM_BLOCKS);
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		degrees[i] = sg.sample();
	}
	for (unsigned int i = 0; i < NUM_BLOCKS; ++i) {
		cout << "i\t" << degrees[i] << endl;
	}
	
	delete[] degrees;
	return 0;
}

