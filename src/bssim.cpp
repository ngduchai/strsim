
/*
 * Simulate the effect of caching and duplication in the system with
 * some low, high variance disks.
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
#include <chrono>
#include "code.h"
#include "store.h"

#define MU 4
#define SIGMA 1

#define NUM_TEST 10000
#define TIME_RANGE 10000
#define BLOCK_RANGE 100
#define BRATE 0.0625 // 64 out of 1024 blocks

#define VISUAL_DLCMF "visual/data/bscmf"
#define VISUAL_DLTL "visual/data/bstl"

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
	
	loadrecord& operator+= (const loadrecord& record) {
		for (unsigned int i = 0; i < TIME_RANGE; ++i) {
			this->restore[i] += record.restore[i];
			this->complete[i] += record.complete[i];
		}
		this->latency.insert(
			this->latency.end(),
			record.latency.begin(),
			record.latency.end()
		);
		return *this;
	}
};

unsigned int NUM_PROC = 0;

void bmsim(unsigned int raw_size, double cache_factor, unsigned int cache_count,
		double dup_factor, unsigned int dup_count, loadrecord ** data,
		unsigned int *arrival, unsigned int * prg) {

	strsim::min_coder coder;
	strsim::gaussian_generator gg(MU, SIGMA);
	strsim::gaussian_generator gl(2*MU, 2*SIGMA);
	
	const unsigned int UNIT_TEST = NUM_TEST / NUM_PROC;


	*prg = 0;
	for (unsigned int cid = 0; cid < cache_count; ++cid) {
		unsigned int cache_size = cid * cache_factor * raw_size;
		for (unsigned int did = 0; did < dup_count; ++did) { 
			unsigned int dup_size = (1 + did * dup_factor) * raw_size;
			loadrecord trail;
			unsigned int num_blocks = dup_size;
			unsigned int num_bl = dup_size * BRATE;
			
			std::vector<strsim::coded_block *> blocks;
			coder.encode(raw_size, num_blocks, blocks);
		
			/*
			std::cout << "Load data with cache_size: " << cache_sz
				", dup_size: " << dup_sz << std::endl;
			*/
			for (unsigned int i = 0; i < UNIT_TEST; ++i) {
				for (unsigned int bid = 0; bid < num_bl; ++bid) {
					blocks[i]->arrieve_time = gl.sample();
				}
				for (unsigned int bid = num_bl; bid < num_blocks; ++bid) {
					blocks[bid]->arrieve_time = gg.sample();
				}
				for (auto block : blocks) {
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
					if (bleft < lastleft) {
						for (time_t j = atime; j < TIME_RANGE; ++j) {
							trail.restore[j] += lastleft - bleft;
						}
						lastleft = bleft;
					}
					if (bleft <= cache_size) {
						trail.latency.push_back(atime);
						for (time_t j = atime; j < TIME_RANGE; ++j) {
							trail.complete[j]++;
						}
						break;
					}
				}
			}
			data[cid][did] = trail;
			for (auto block : blocks) {
				delete block;
			}
		}
		(*prg)++;
	}
}

int main(int argc, char ** argv) {
	if (argc != 6) {
		std::cerr << "Usage: simplesim [raw_size] " <<
			"[cache_factor] [max_cachefactor] " <<
			"[dup_factor] [num_dupfactor]" << std::endl;
		return 1;
	}
	
	const unsigned int RAW_SIZE = std::stoi(argv[1]);
	const double CACHE_FACTOR = std::stod(argv[2]);
	const double MAX_CACHE = std::stod(argv[3]);
	const double DUP_FACTOR = std::stod(argv[4]);
	const double MAX_DUP = std::stod(argv[5]);

	unsigned int num_cache = MAX_CACHE / CACHE_FACTOR + 1;
	unsigned int num_dup = MAX_DUP / DUP_FACTOR + 1;

	NUM_PROC = std::thread::hardware_concurrency() / 2;
	std::cout << "Number of processors: " << NUM_PROC << std::endl;

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
	
	std::thread * processors = new std::thread [NUM_PROC];
	for (unsigned int i = 0; i < NUM_PROC; ++i) {
		processors[i] = std::thread(bmsim, RAW_SIZE, CACHE_FACTOR, num_cache,
				DUP_FACTOR, num_dup, data[i], arrival[i], &prg[i]);
	}

	bool done = false;
	unsigned int min_prg = 0;
	if (num_cache <= 1){
		done = true;
	}
	
	while (!done) {
		unsigned int mprg = num_cache;
		for(unsigned int i = 0; i < NUM_PROC; ++i) {
			if (prg[i] < mprg) {
				mprg = prg[i];
			}
		}
		if (mprg > min_prg) {
			min_prg = mprg;
			std::cout << "Progress: " <<
				(double)min_prg / (double)num_cache * 100 <<
				"%" << std::endl;
		}
		if (min_prg == num_cache - 1) {
			done = true;
		}else{
			std::this_thread::sleep_for(
				std::chrono::milliseconds(1000)
			);
		}
	}

	for (unsigned int i = 0; i < NUM_PROC; ++i) {
		processors[i].join();
	}
	delete [] processors;

	/* Calculate results */
	for (unsigned int i = 1; i < NUM_PROC; ++i) {
		for (unsigned int j = 0; j < num_cache; ++j) {
			for (unsigned int k = 0; k < num_dup; ++k) {
				data[0][j][k] += data[i][j][k];
			}
		}
		for (unsigned int j = 0; j < TIME_RANGE; ++j) {
			arrival[0][j] += arrival[i][j];
		}
	} 

	std::ofstream report_cmf(VISUAL_DLCMF);
	report_cmf << "Time,Arrival,";
	for (unsigned int i = 0; i < num_cache; ++i) {
		for (unsigned int j = 0; j < num_dup; ++j) {
			report_cmf <<
				"C=" << (i*CACHE_FACTOR) << "N-" << 
				"K=" << (j*DUP_FACTOR) << "N,";
			std::sort(
				data[0][i][j].latency.begin(),
				data[0][i][j].latency.end()
			);
		}
	}
	report_cmf << std::endl;
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		report_cmf << double(i) / 1000 << "," <<
			double(arrival[0][i]) /
			double(arrival[0][TIME_RANGE-1]) << ",";
		for (unsigned int cid = 0; cid < num_cache; ++cid) {
			for (unsigned int did = 0; did < num_dup; ++did) {
				report_cmf <<
					double(data[0][cid][did].complete[i]) /
					double(data[0][cid][did].complete[TIME_RANGE-1]) << ",";
			}
		}
		report_cmf << std::endl;
	}
	
	std::ofstream report_dl(VISUAL_DLTL);
	unsigned tid = NUM_TEST / 100 * 99;
	report_dl << "C,K,avg_latency,tail_latency" << std::endl;
	for (unsigned int i = 0; i < num_cache; ++i) {
		for (unsigned int j = 0; j < num_dup; ++j) {
			report_dl << i*CACHE_FACTOR << "," << j*DUP_FACTOR << "," <<
				data[0][i][j].avg_latency() << "," <<
				data[0][i][j].latency[tid] << "," << std::endl;
		}
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

