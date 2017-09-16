#include <iostream>
#include <algorithm>
#include <fstream>
#include <list>
#include <string>
#include "code.h"
#include "store.h"

#define SHAPE 3
#define RATE 2.0
#define DELAY 1.0

#define NUM_TEST 10000
#define TIME_RANGE 10000
#define BLOCK_RANGE 100

#define VISUAL_DIST "visual/data/dist"
#define VISUAL_CMF "visual/data/cmf"
#define VISUAL_FR "visual/data/fr"
#define VISUAL_TAIL "visual/data/tail"
#define VISUAL_HEAD "visual/data/head"
#define VISUAL_MID "visual/data/mid"
#define VISUAL_SCALARS "visual/data/scalars"

struct loadrecord {
	// blocks used for reconstruct the original data
	std::vector<strsim::coded_block*> blocks;
	time_t ftime; 	// Original latency
	time_t rtime; 	// Latency of caching the several last blocks
	time_t mtime; 	// Latency of trying to cache and use block from
					// the very beginning
	// list of block left when loading data
	std::list<unsigned int> blockleft;
};

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
	
	strsim::luby_coder coder;
	strsim::erlang_generator eg(SHAPE, RATE, DELAY);
	
	// number of block arrival after a certain point of time
	unsigned long * arrival = new unsigned long [TIME_RANGE];
	// number of block constructed after a certain point of time
	unsigned long * complete = new unsigned long [TIME_RANGE];
	// number of block on the cache use for reconstruction
	// after a certain point of time
	unsigned long * rcache = new unsigned long [TIME_RANGE];
	// number of original data restored after a certain point
	// of time
	unsigned long * constructed = new unsigned long [TIME_RANGE];
	// Assume we restore block N-th and (N+1)-th at t0 and t1,
	// then we want to know the number of blocks loaded between
	// t0 and t1 
	unsigned long * nwait = new unsigned long [BLOCK_RANGE];
	// The number of block we can reconstruct if we successfully
	// fetch a new block from storage system
	unsigned long * nrestore = new unsigned long [BLOCK_RANGE];
	for (int i = 0; i < TIME_RANGE; ++i) {
		arrival[i] = 0;
		complete[i] = 0;
		rcache[i] = 0;
		constructed[i] = 0;
	}
	for (int i = 0; i < BLOCK_RANGE; ++i) {
		nwait[i] = 0;
		nrestore[i] = 0;
	}

	std::vector<loadrecord> data;
	std::cout << "Loading data" << std::endl;
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		loadrecord trail;
		if ((i+1) % (NUM_TEST / 10) == 0) {
			std::cout << "Processed " <<
				(i+1) / (NUM_TEST / 100) << "%" << std::endl;
		}
		coder.encode(RAW_SIZE, CODED_SIZE, trail.blocks);
		for (auto block : trail.blocks) {
			block->arrieve_time = eg.sample();
			if (block->arrieve_time >= TIME_RANGE) {
				continue;
			}
			for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
				arrival[j]++;
			}
		}
		std::sort(trail.blocks.begin(), trail.blocks.end(),
				[] (strsim::coded_block* f,
				strsim::coded_block* s) -> bool {
					return (f->arrieve_time < s->arrieve_time);
				});
		coder.restart();
		bool wait = false;
		trail.mtime = trail.blocks[RAW_SIZE - CACHED_SIZE]->arrieve_time;
		unsigned int lastleft = RAW_SIZE;
		unsigned int waitcount = 0;
		for (auto block : trail.blocks) {
			unsigned int bleft = coder.decode(block);
			trail.blockleft.push_back(bleft);
			if (bleft < lastleft) {
				for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
					complete[j] += lastleft - bleft;
				}
				if (lastleft - bleft - 1 < BLOCK_RANGE) {
					nrestore[lastleft - bleft - 1]++;
				}else{
					nrestore[BLOCK_RANGE - 1]++;
				}
				if (waitcount < BLOCK_RANGE) {
					nwait[waitcount]++;
				}else{
					nwait[BLOCK_RANGE-1]++;
				}
				waitcount = 0;
				lastleft = bleft;
			}else{
				waitcount++;
			}
			if (bleft <= CACHED_SIZE && !wait) {
				trail.rtime = block->arrieve_time;
				wait = true;
				for (unsigned int j = block->arrieve_time;
						j < TIME_RANGE; ++j) {
					rcache[j] += CACHED_SIZE;
				}
			}
			if (coder.has_finished()) {
				for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
					constructed[j]++;
				}
				trail.ftime = block->arrieve_time;
				break;
			}
		}
		data.push_back(trail);
	}
	double tf = 0;
	double tr = 0;
	double trm = 0;
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		tf += data[i].ftime;
		tr += data[i].rtime;
		trm += data[i].mtime;
	}
	std::cout << "avg ftime = " << tf << std::endl;
	std::cout << "avg rtime = " << tr << std::endl;
	std::cout << "avg mtime = " << trm << std::endl;
	std::cout << "Gain rtime = " << double(tf - tr) / tf << std::endl;	
	std::cout << "Gain mtime = " << double(tf - trm) / tf << std::endl;	

	std::sort(data.begin(), data.end(),
			[] (loadrecord& f, loadrecord& s) -> bool {
				return (f.rtime < s.rtime);
			});
	time_t rt = data[NUM_TEST / 100 * 99].rtime;
	std::sort(data.begin(), data.end(),
			[] (loadrecord& f, loadrecord& s) -> bool {
				return (f.mtime < s.mtime);
			});
	time_t mt = data[NUM_TEST / 100 * 99].mtime;
	std::sort(data.begin(), data.end(),
			[] (loadrecord& f, loadrecord& s) -> bool {
				return (f.ftime < s.ftime);
			});
	time_t ft = data[NUM_TEST / 100 * 99].ftime;
	std::cout << "99-th ftime = " << ft << std::endl;
	std::cout << "99-th rtime = " << rt << std::endl;
	std::cout << "99-th mtime = " << mt << std::endl;
	std::cout << "Gain rtime = " << double(ft - rt) / ft << std::endl;	
	std::cout << "Gain mtime = " << double(ft - mt) / ft << std::endl;
	/* Write the progress of 99-th trails */
	std::string prhead =
		"No. blocks fetched, No. blocks reconstructed"; 
	std::ofstream report_progress(VISUAL_TAIL);
	unsigned int tright = 0;
	report_progress << prhead << std::endl;
	for (unsigned int bleft : data[NUM_TEST / 100 * 99].blockleft) {
		report_progress << tright << "," << bleft << std::endl;
		tright++;
	}
	report_progress.close();
	report_progress.open(VISUAL_HEAD);
	report_progress << prhead << std::endl;
	tright = 0;
	for (unsigned int bleft : data[NUM_TEST / 100 * 5].blockleft) {
		report_progress << tright << "," << bleft << std::endl;
		tright++;
	}
	report_progress.close();
	report_progress.open(VISUAL_MID);
	report_progress << prhead << std::endl;
	tright = 0;
	for (unsigned int bleft : data[NUM_TEST / 100 * 50].blockleft) {
		report_progress << tright << "," << bleft << std::endl;
		tright++;
	}
	report_progress.close();

	/* Write arrival distribution to output file */
	std::ofstream report_dist(VISUAL_DIST);
	std::ofstream report_cmf(VISUAL_CMF);
	std::ofstream report_fr(VISUAL_FR);
	std::string sthead = "Time, \% of blocks arrived,"
		"\% of blocks constructed, Finish, Finish w. caching";
	report_cmf << sthead << std::endl;
	for (time_t i = 0; i < TIME_RANGE; ++i) {
		report_cmf << double(i) / 1000 << "," <<
			double(arrival[i]) / double(arrival[TIME_RANGE-1]) << "," <<
			double(complete[i]) / double(complete[TIME_RANGE-1]) << "," <<
			double(rcache[i]) / double(rcache[TIME_RANGE-1]) << "," <<
			double(constructed[i]) /
			double(constructed[TIME_RANGE-1]) << "," <<
			std::endl;
	}
	report_dist << sthead << std::endl;
	for (time_t i = 1; i < TIME_RANGE; ++i) {
		report_dist << double(i) / 1000 << "," <<
			double(arrival[i] - arrival[i-1]) /
			double(arrival[TIME_RANGE-1]) * TIME_RANGE << "," <<
			double(complete[i] - complete[i-1]) /
			double(complete[TIME_RANGE-1]) * TIME_RANGE << "," <<
			double(rcache[i] - rcache[i-1]) /
			double(rcache[TIME_RANGE-1]) << "," <<
			double(constructed[i] - constructed[i-1]) /
			double(constructed[TIME_RANGE-1]) << "," <<
			std::endl;	
	}
	report_fr << "Num, No. Block per Interval, Num. Block Restored" <<
		std::endl;
	for (time_t i = 0; i < BLOCK_RANGE; ++i) {
		report_fr << i + 1 << "," <<
			nwait[i] << "," << nrestore[i] << std::endl;
	}
	report_dist.close();
	report_cmf.close();
	report_fr.close();
	delete[] arrival;
	delete[] complete;
	delete[] rcache;
	delete[] nwait;
	delete[] nrestore;

	/* Rerun the last 10% of trails to investigate the case */
	arrival = new unsigned long [TIME_RANGE];
	complete = new unsigned long [TIME_RANGE];
	constructed = new unsigned long [TIME_RANGE];
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		arrival[i] = 0;
		complete[i] = 0;
		constructed[i] = 0;
	}
	unsigned int ttl = 10;
	while (ttl > 0) {
		//for (unsigned int i = NUM_TEST / 100 * 90; i < NUM_TEST; ++i) {
		for (unsigned int i = 0; i < NUM_TEST / 10; ++i) {
			for (auto block : data[i].blocks) {
				block->arrieve_time = eg.sample();
				if (block->arrieve_time >= TIME_RANGE) {
					continue;
				}
				for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
					arrival[j]++;
				}
			}
			std::sort(data[i].blocks.begin(), data[i].blocks.end(),
					[] (strsim::coded_block* f,
						strsim::coded_block* s) -> bool {
					return (f->arrieve_time < s->arrieve_time);
					});
			coder.restart();
			unsigned int lastleft = RAW_SIZE;
			for (auto block : data[i].blocks) {
				unsigned int bleft = coder.decode(block);
				if (bleft < lastleft) {
					for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
						complete[j] += lastleft - bleft;
					}
					lastleft = bleft;
				}
				if (coder.has_finished()) {
					for (time_t j = block->arrieve_time; j < TIME_RANGE; ++j) {
						constructed[j]++;
					}
					break;
				}
			}
		}
		ttl--;
	}
	report_cmf.open("visual/cmftail");
	report_cmf << "Time, Arrival, Complete, Constructed" << std::endl;
	for (time_t i = 0; i < TIME_RANGE; ++i) {
		report_cmf << double(i) / 1000 << "," <<
			double(arrival[i]) / double(arrival[TIME_RANGE-1]) << "," <<
			double(complete[i]) / double(complete[TIME_RANGE-1]) << "," <<
			double(constructed[i]) / double(constructed[TIME_RANGE-1]) << "," <<
			std::endl;	
	}
	report_cmf.close();

	return 0;
}

