#include <iostream>
#include <algorithm>
#include <fstream>
#include "code.h"
#include "store.h"

#define SHAPE 3
#define RATE 2.0
#define DELAY 1.0

#define NUM_TEST 10000
#define TIME_RANGE 10000

#define VISUAL_DIST "visual/dist"
#define VISUAL_SCALARS "visual/scalars"

int main(int argc, char ** argv) {
	if (argc != 4) {
		std::cerr << "Usage: simplesim [raw_size] " <<
			"[dup_factor] [cache_factor]" << std::endl;
		return 1;
	}
	const unsigned int RAW_SIZE = std::stoi(argv[1]);
	const double DUP_FACTOR = std::stod(argv[2]);
	const double CACHE_FACTOR = std::stod(argv[3]);
	
	const unsigned int CODED_SIZE = RAW_SIZE * DUP_FACTOR;
	const unsigned int CACHED_SIZE  = RAW_SIZE * CACHE_FACTOR;
	
	std::vector<strsim::coded_block*> blocks;
	//strsim::rateless_coder coder;
	strsim::luby_coder coder;
	strsim::erlang_generator eg(SHAPE, RATE, DELAY);
	std::vector<time_t> ftime;
	std::vector<time_t> rtime;
	std::vector<time_t> mtime;

	unsigned long * arrival = new unsigned long [TIME_RANGE];
	unsigned long * complete = new unsigned long [TIME_RANGE];
	unsigned long * rcache = new unsigned long [TIME_RANGE];
	for (int i = 0; i < TIME_RANGE; ++i) {
		arrival[i] = 0;
		complete[i] = 0;
		rcache[i] = 0;
	}
	
	std::cout << "Loading data" << std::endl;
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		if ((i+1) % (NUM_TEST / 10) == 0) {
			std::cout << "Processed " <<
				(i+1) / (NUM_TEST / 100) << "%" << std::endl;
		}
		coder.encode(RAW_SIZE, CODED_SIZE, blocks);
		for (auto block : blocks) {
			block->arrieve_time = eg.sample();
			if (block->arrieve_time >= TIME_RANGE) {
				continue;
			}
			for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
				arrival[j]++;
			}
		}
		std::sort(blocks.begin(), blocks.end(), [] (strsim::coded_block* f,
				strsim::coded_block* s) -> bool {
					return (f->arrieve_time < s->arrieve_time);
				});
		coder.restart();
		bool wait = false;
		mtime.push_back(blocks[RAW_SIZE - CACHED_SIZE]->arrieve_time);
		unsigned int lastleft = RAW_SIZE;
		for (auto block : blocks) {
			unsigned int bleft = coder.decode(block);
			if (bleft < lastleft) {
				for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
					complete[j] += lastleft - bleft;
				}
				lastleft = bleft;
			}
			if (bleft <= CACHED_SIZE && !wait) {
				rtime.push_back(block->arrieve_time);
				wait = true;
				for (unsigned int j = block->arrieve_time;
						j < TIME_RANGE; ++j) {
					rcache[j] += CACHED_SIZE;
				}
			}
			if (coder.has_finished()) {
				ftime.push_back(block->arrieve_time);
				break;
			}
		}

	}
	double tf = 0;
	double tr = 0;
	double trm = 0;
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		tf += ftime[i];
		tr += rtime[i];
		trm += mtime[i];
	}
	std::cout << "avg ftime = " << tf << std::endl;
	std::cout << "avg rtime = " << tr << std::endl;
	std::cout << "avg mtime = " << trm << std::endl;
	std::cout << "Gain rtime = " << double(tf - tr) / tf << std::endl;	
	std::cout << "Gain mtime = " << double(tf - trm) / tf << std::endl;	

	std::sort(ftime.begin(), ftime.end(), [] (time_t f, time_t s) -> bool {
				return (f < s);
			});
	std::sort(rtime.begin(), rtime.end(), [] (time_t f, time_t s) -> bool {
				return (f < s);
			});
	std::sort(mtime.begin(), mtime.end(), [] (time_t f, time_t s) -> bool {
				return (f < s);
			});
	time_t ft = ftime[NUM_TEST / 100 * 99];
	time_t rt = rtime[NUM_TEST / 100 * 99];
	time_t mt = mtime[NUM_TEST / 100 * 99];
	std::cout << "99-th ftime = " << ft << std::endl;
	std::cout << "99-th rtime = " << rt << std::endl;
	std::cout << "99-th mtime = " << mt << std::endl;
	std::cout << "Gain rtime = " << double(ft - rt) / ft << std::endl;	
	std::cout << "Gain mtime = " << double(ft - mt) / ft << std::endl;
	/* Write arrival distribution to output file */
	std::ofstream report(VISUAL_DIST);
	for (time_t i = 0; i < TIME_RANGE; ++i) {
		report << double(i) / 1000 << "," <<
			double(arrival[i]) / double(arrival[TIME_RANGE-1]) << "," <<
			double(complete[i]) / double(complete[TIME_RANGE-1]) << "," <<
			double(rcache[i]) / double(rcache[TIME_RANGE-1]) << "," <<
			std::endl;
	}
	report.close();
	delete[] arrival;
	delete[] complete;
	delete[] rcache;
	return 0;
}
