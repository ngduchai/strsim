#include <iostream>
#include <list>
#include <vector>
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

#define VISUAL_DLCMF "visual/data/dlcmf"
#define VISUAL_DLTL "visual/data/dltl"

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
	if (argc != 5) {
		std::cerr << "Usage: simplesim [raw_size] " <<
			"[dup_factor] [max_dupfactor] [cache_factor]" << std::endl;
		return 1;
	}
	const unsigned int RAW_SIZE = std::stoi(argv[1]);
	const double DUP_FACTOR = std::stod(argv[2]);
	const double MAX_DUPFACTOR = std::stod(argv[3]);
	const double CACHE_FACTOR = std::stod(argv[4]);
	
	const unsigned int CACHED_SIZE = RAW_SIZE * CACHE_FACTOR;
	const unsigned int MAXDUP = RAW_SIZE * MAX_DUPFACTOR;
	unsigned int num_blocks = RAW_SIZE;
	unsigned long * arrival = new unsigned long [TIME_RANGE];
	
	strsim::min_coder coder;
	//strsim::erlang_generator eg(SHAPE, RATE, DELAY);
	strsim::gaussian_generator gg(4.0, 1.0);

	std::list<loadrecord> data;
	std::list<loadrecord> cdata;

	while (num_blocks <= MAXDUP) {
		loadrecord trail;
		loadrecord ctrail;
		std::cout << "Load data with extra " <<
			double(num_blocks - RAW_SIZE) / RAW_SIZE * 100 <<
			"%" << std::endl;
		for (unsigned int i = 0; i < NUM_TEST; ++i) {
			std::vector<strsim::coded_block *> blocks;
			coder.encode(RAW_SIZE, num_blocks, blocks);
			for (auto block : blocks) {
				//block->arrieve_time = eg.sample();
				block->arrieve_time = gg.sample();
				if (block->arrieve_time >= TIME_RANGE) {
					continue;
				}
				for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
					arrival[j]++;
				}
			}
			std::sort(blocks.begin(), blocks.end(),
				[] (strsim::coded_block* f,
						strsim::coded_block *s) -> bool {
					return (f->arrieve_time < s->arrieve_time);
			});
			coder.restart();
			unsigned int lastleft = RAW_SIZE;
			bool wait = true;
			for (auto block : blocks) {
				unsigned int bleft = coder.decode(block);
				time_t atime = block->arrieve_time;
				delete block;
				if (bleft > lastleft) {
					for (time_t j = atime; j < TIME_RANGE; ++j) {
						trail.restore[j] += lastleft - bleft;
						ctrail.restore[j] += lastleft - bleft;
					}
				}
				if (bleft <= CACHED_SIZE && wait) {
					ctrail.latency.push_back(atime);
					for (time_t j = atime; j < TIME_RANGE; ++j) {
						ctrail.complete[j]++;
					}
					wait = false;
				}
				if (coder.has_finished()) {
					trail.latency.push_back(atime);
					for (time_t j = atime; j < TIME_RANGE; ++j) {
						trail.complete[j]++;
					}
					break;
				}
			}
		}
		data.push_back(trail);
		cdata.push_back(ctrail);
		num_blocks += (unsigned int)(RAW_SIZE * DUP_FACTOR);
	}

	std::ofstream report_cmf(VISUAL_DLCMF);
	report_cmf << "Time,Arrival,";
	unsigned int factor = 0;
	for (auto& record : data) {
		report_cmf << "mn-" << factor << ",";
		std::sort(record.latency.begin(), record.latency.end());
		factor += RAW_SIZE * DUP_FACTOR;
	}
	factor = 0;
	for (auto& record : cdata) {
		report_cmf << "cachemn-" << factor << ",";
		std::sort(record.latency.begin(), record.latency.end());
		factor += RAW_SIZE * DUP_FACTOR;
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
		for (auto& record : cdata) {
			report_cmf << double(record.complete[i]) /
				double(record.complete[TIME_RANGE-1]) << ",";
		}
		report_cmf << std::endl;
	}
	
	std::ofstream report_dl(VISUAL_DLTL);
	auto ltdata = data.begin();
	auto ltcdata = cdata.begin();
	unsigned tid = NUM_TEST / 100 * 99;
	report_dl << "extra," <<
		"avg_latency,cache_avg_latency," << 
		"tail_latency,cache_tail_latency" << std::endl;
	for (unsigned int i = 0; i < MAXDUP - RAW_SIZE;
			i += RAW_SIZE*DUP_FACTOR) {
		report_dl << i << "," <<
			ltdata->avg_latency() << "," << ltcdata->avg_latency() << "," <<
			ltdata->latency[tid] << "," <<
			ltcdata->latency[tid] << std::endl;
		ltdata++;
		ltcdata++;
	}

	report_cmf.close();
	report_dl.close();
}

