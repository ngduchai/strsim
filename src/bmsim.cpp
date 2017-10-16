
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
#include <fstream>
#include <algorithm>
#include <thread>
#include "code.h"
#include "store.h"

#define MU 4
#define SIGMA 2

#define NUM_TEST 10000
#define TIME_RANGE 10000
#define BLOCK_RANGE 100

#define VISUAL_DLCMF "visual/data/mmcmf"
#define VISUAL_DLTL "visual/data/mmtl"

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
	
	loadrecord& operator+ (const loadrecord& record) {
		for (unsigned int i = 0; i < TIME_RANGE; ++i) {
			this->restore[i] += record.restore[i];
			this->complete[i] += record.complete[i];
		}
		this->latency.insert(
			this->latency.end(),
			record->latency.begin(),
			record->latency.end()
		);
	}
};

void bmsim(unsigned int raw_size, double cache_factor, unsigned int max_cache,
		double dup_factor, unsigned int max_dup, loadrecord ** data,
		unsigned int *arrival, int * prg) {

	strsim::min_coder coder;
	strsim::gaussian_generator gg(MU, SIGMA);
	
	const unsigned int NUM_PROC = std::thread::hardware_concurrency();
	const unsigned int UNIT_TEST = NUM_TEST / NUM_PROC;
	
	unsigned int cache_size = 0;
	unsigned int cid = 0;
	prg = 0;
	while (cache_size < max_cache) {
		unsigned int dup_size = 0;
		unsigned int did = 0;
		while (dup_size < max_dup) { 
			loadrecord trail;

			/*
			std::cout << "Load data with cache_size: " << cache_sz
				", dup_size: " << dup_sz << std::endl;
			*/
			for (unsigned int i = 0; i < UNIT_TEST; ++i) {
				std::vector<strsim::coded_block *> blocks;
				coder.encode(RAW_SIZE, num_blocks, blocks);
				for (auto block : blocks) {
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
				unsigned int lastleft = raw_size;
				for (auto block : blocks) {
					unsigned int bleft = coder.decode(block);
					time_t atime = block->arrieve_time;
					delete block;
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
				}
			}
			data[cid][did] = trail;
			did++;
			dup_size = (1 + did * dup_factor) * raw_size;
		}
		cid++;
		cache_size = cid * cache_factor * raw_size;
		(*prg)++;
	}
}

int main(int argc, char ** argv) {
	if (argc != 5) {
		std::cerr << "Usage: simplesim [raw_size] " <<
			"[cache_factor] [max_cachefactor] " <<
			"[dup_factor] [max_dupfactor]" << std::endl;
		return 1;
	}
	
	const unsigned int RAW_SIZE = std::stoi(argv[1]);
	const double CACHE_FACTOR = std::stod(argv[2]);
	const unsigned int MAX_CACHE = std::stod(argv[3]) * RAW_SIZE;
	const double DUP_FACTOR = std::stod(argv[4]);
	const unsigned MAX_DUP = std::stod(argv[5]) * RAW_SIZE;

	unsigned int num_cache = MAX_CACHE / CACHE_FACTOR + 1;
	unsigned int num_dup = MAX_DUP / DUP_FACTOR + 1;

	const unsigned int NUM_PROC = std::thread::hardware_concurrency();

	/* Initialize space for putting results */
	loadrecord *** data = new loadrecord ** [NUM_PROC];
	unsigned int * prg = new unsigned int [NUM_PROC];
	unsigned int ** arrival = new unsigned int * [NUM_PROC];
	for (unsigned int i = 0; i < NUM_PROC; ++i) {
		data[i] = new loadrecord * [num_cache];
		for (unsigned int j = 0; j < num_cache; ++j) {
			data[i][j] = new loadrecord [num_dup];
		}
		arrival[i] = new unsigned int [TIME_RANGE];
	}
	std::cout << "Run simulation" << std::endl;
	
	std::thread processors = new thread [NUM_PROC];
	for (unsigned int i = 0; i < NUM_PROC; ++i) {
		processors[i] = std::thread(bmsim, RAW_SIZE, CACHE_FACTOR, MAX_CACHE,
				DUP_FACTOR, MAX_DUP, data[i], arrival[i], &prg[i]);
	}

	bool done = false;
	unsigned int min_prg = num_cache;
	while (!done) {
		unsigned int mprg = min_prg;
		for(unsigned int i = 0; i < NUM_PROC; ++i) {
			if (prg[i] < mprg) {
				mprg = prg[i];
			}
		}
		if (mprg < min_prg) {
			std::cout << "Progress: " << min_prg / num_cache * 100 <<
				"%" << std:endl;
		}
		if (min_prg == num_cache) {
			done = true;
		}
	}

	for (unsigned int i = 0; i < NUM_PROC; ++i) {
		processors[i].join();
	}
	delete [] processors;

	/* Calculate results */
	for (unsigned int i = 1; i < NUM_PROC; ++i) {
		data[0] += data[i];
	} 

	std::ofstream report_cmf(VISUAL_DLCMF);
	report_cmf << "Time,Arrival,";
	unsigned int factor = 0;
	for (unsigned int i = 0; i < num_cache; ++i) {
		for (unsigned int j = 0; j < num_dup; ++j) {
			report_cmf <<
				"C = " << (int)(i*cache_factor) << "N - " << 
				"K = " << (int)(j*dup_factor) << "N,";
			std::sort(
				data[0][i][j].latency.begin(),
				data[0][i][j].latency.end()
			);
		}
	}
	report_cmf << std::endl;
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		report_cmf << double(i) / 1000 << "," <<
			double(arrival[i]) /
			double(arrival[TIME_RANGE-1]) << ",";
		for (unsigned int cid = 0; cid < num_cache; ++cid) {
			for (unsigned int did = 0; did < num_dup; ++did) {
				report_cmf <<
					double(data[0][cid][did].complete[i]) /
					double(data[cid][did].complete[TIME_RANGE-1]) << ",";
		}
		report_cmf << std::endl;
	}
	
	std::ofstream report_dl(VISUAL_DLTL);
	unsigned tid = NUM_TEST / 100 * 99;
	report_dl << "C,K,avg_latency,tail_latency" << std::endl;
	for (unsigned int i = 0; i < num_cache; ++i) {
		for (unsigned int j = 0; j < num_dup; ++j) {
			report_dl << i*cache_factor << "," << j*dup_factor << "," <<
				data[0][i][j].avg_latency() << "," <<
				data[0][i][j].latency[tid] << "," << std::endl;
	}

	report_cmf.close();
	report_dl.close();
	
	for (unsigned int i = 0; i < NUM_PROC; ++i) {
		for (unsigned int j = 0; j < num_cache; ++j) {
			delete [] data[i][j];
		}
		delete [] data[i];
		delete [] arrival[i];
	}
	delete [] data;
	delete [] prg;
	delete [] arrival;


	

}

