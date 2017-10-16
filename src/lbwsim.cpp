
/*
 * Simulate the effect of caching with a fix amount of duplication
 * Input:
 * 	- raw_size
 * 	- dup_factod
 * 	- cache_factor (step of increment)
 * 	- max_cachefactor
 * Output:
 *  - The CMF of arrival time
 *  - The CMF of completion time with different amount of caching
 *  - The average and tail latency with diffrent amount of caching
 *
 * */

#include <iostream>
#include <list>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include "code.h"
#include "store.h"

#define SHAPE 3
#define RATE 2.0
#define DELAY 1.0

#define NUM_TEST 10000
#define TIME_RANGE 10000
#define BLOCK_RANGE 100

#define VISUAL_DLCMF "visual/data/lbwcmf"
#define VISUAL_DLTL "visual/data/lbwtl"

class blockcomp {
public:
	bool operator()(strsim::coded_block * f, strsim::coded_block * s) {
		return f->arrieve_time > s->arrieve_time;
	}
};

struct loadrecord {
	unsigned long restore [TIME_RANGE];
	unsigned long complete [TIME_RANGE];
	std::vector<time_t> latency;
	loadrecord() {
		for (int i = 0; i < TIME_RANGE; ++i) {
			restore[i] = complete[i] = 0;
		}
	}
	time_t avg_latency() {
		time_t sumlt = 0;
		for (time_t lt : latency) {
			sumlt += lt;
		}
		return sumlt / latency.size();
	}
};

int main(int argc, char ** argv) {
	if (argc != 6) {
		std::cerr << "Usage: simplesim [raw_size] [cache_factor] " << 
			"[max_cachefactor] [dup_factor] [load_factor]" << std::endl;
		return 1;
	}
	const unsigned int RAW_SIZE = std::stoi(argv[1]);
	const double CACHE_FACTOR = std::stod(argv[2]);
	const double MAX_CACHEFACTOR = std::stod(argv[3]);
	const double DUP_FACTOR = std::stod(argv[4]);
	const double LOAD_FACTOR = std::stod(argv[5]);
	
	const unsigned int MAXCACHE = RAW_SIZE * MAX_CACHEFACTOR;
	unsigned int cached_size = 0;
	unsigned int num_blocks = RAW_SIZE * DUP_FACTOR;
	unsigned int num_load = RAW_SIZE * LOAD_FACTOR;
	unsigned long * arrival = new unsigned long [TIME_RANGE];
	
	strsim::min_coder coder;
	//strsim::erlang_generator eg(SHAPE, RATE, DELAY);
	strsim::gaussian_generator gg(3.0, 2.0);

	std::list<loadrecord> data;

	while (cached_size <= MAXCACHE) {
		loadrecord trail;
		std::cout << "Load data with cache size " <<
			cached_size << std::endl;
		for (unsigned int i = 0; i < NUM_TEST; ++i) {
			std::vector<strsim::coded_block *> blocks;
			std::priority_queue<strsim::coded_block*,
				std::vector<strsim::coded_block*>, blockcomp> block_queue;
			coder.encode(RAW_SIZE, num_blocks, blocks);
			for (unsigned int i = 0; i < num_load; ++i) {
				blocks[i]->arrieve_time = gg.sample();
				block_queue.push(blocks[i]);
			}
			coder.restart();
			unsigned int lastleft = RAW_SIZE;
			unsigned int cid = num_load;
			while (cid < blocks.size()) {
				strsim::coded_block * block = block_queue.top();
				unsigned int bleft = coder.decode(block);
				time_t atime = block->arrieve_time;
				//delete block;
				if (bleft < lastleft) {
					for (time_t j = atime; j < TIME_RANGE; ++j) {
						trail.restore[j] += lastleft - bleft;
					}
					lastleft = bleft;
				}
				if (bleft <= cached_size) {
					trail.latency.push_back(atime);
					for (time_t j = atime; j < TIME_RANGE; ++j) {
						trail.complete[j]++;
					}
					break;
				}
				block_queue.pop();
				blocks[cid]->arrieve_time = atime + gg.sample();
				block_queue.push(blocks[cid]);
				cid++;
			}
			for (unsigned int j = 0; j < cid; ++j) {
				for (time_t k = blocks[j]->arrieve_time;
						k < TIME_RANGE; ++k) {
					arrival[k]++;
				}
			}
		}
		data.push_back(trail);
		cached_size += (unsigned int)(RAW_SIZE * CACHE_FACTOR);
	}

	std::ofstream report_cmf(VISUAL_DLCMF);
	report_cmf << "Time,Arrival,";
	unsigned int factor = 0;
	for (auto& record : data) {
		report_cmf << factor << ",";
		std::sort(record.latency.begin(), record.latency.end());
		factor += RAW_SIZE * CACHE_FACTOR;
	}
	report_cmf << std::endl;
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		report_cmf << double(i) / 1000 << "," <<
			double(arrival[i]) /
			double(arrival[TIME_RANGE-1]) << ",";
		for (auto& record : data) {
			report_cmf << double(record.complete[i]) /
				double(record.complete[TIME_RANGE-1]) << ",";
		}
		report_cmf << std::endl;
	}
	
	std::ofstream report_dl(VISUAL_DLTL);
	auto ltdata = data.begin();
	unsigned tid = NUM_TEST / 100 * 99;
	report_dl << "extra,avg_latency,tail_latency" << std::endl;
	for (unsigned int i = 0; i <= MAXCACHE;
			i += RAW_SIZE*CACHE_FACTOR) {
		report_dl << i << "," <<
			ltdata->avg_latency() << "," <<
			ltdata->latency[tid] << "," << std::endl;
		ltdata++;
	}

	report_cmf.close();
	report_dl.close();
}

