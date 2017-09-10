#include <iostream>
#include <vector>
#include <random>
#include "code.h"

using namespace std;
using namespace strsim;

#define RAW_BLOCK 20
#define CODED_BLOCK 40

int main() {
	rateless_coder coder;
	vector<coded_block*> blocks;
	coder.encode(RAW_BLOCK, CODED_BLOCK, blocks);
	cout << "Generate " << CODED_BLOCK << " coded block(s) from " <<
		RAW_BLOCK << " raw block(s):" << endl;
	for (auto b : blocks) {
		rateless_block *block = static_cast<rateless_block*>(b);
		for (auto raw : block->raw_blocks) {
			cout << raw << ", ";
		}
		cout << endl;
	}
	cout << "Restore raw block from coded block" << endl;
	std::default_random_engine gen;
	std::uniform_int_distribution<int> dist(0, CODED_BLOCK - 1);
	for (unsigned int i = 0; i < CODED_BLOCK / 2; ++i) {
		int source = dist(gen);
		int destiation = dist(gen);
		swap(blocks[source], blocks[destiation]);
	}
	coder.restart();
	for (auto b : blocks) {
		rateless_block * block = static_cast<rateless_block*>(b);
		for (auto raw : block->raw_blocks) {
			cout << raw << ", ";
		}
		cout << endl;
		coder.decode(block);
		if (coder.has_finished()) {
			break;
		}
	}
	for (auto block : blocks) {
		delete static_cast<rateless_block*>(block);
	}
	blocks.clear();
}


