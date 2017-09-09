#include "code.h"
#include "iostream"

void strsim::soliton_generator::setup(
		strsim::degree_generator::value_type seed) {
	using value_type = strsim::degree_generator::value_type;
	// build the distribution table
	delete[] _cdf;
	_size = seed + 1;
	_cdf = new double [seed + 1];
	_cdf[0] = 0;
	_cdf[1] = 1.0 / seed;
	for (value_type i = 2; i <= seed; ++i) {
		_cdf[i] = _cdf[i-1] + 1.0 / (i * (i-1));
	}
	/*
	for (value_type i = 0; i <= seed; ++i) {
		DBG(_cdf[i]);
	}
	*/
}

strsim::degree_generator::value_type strsim::soliton_generator::sample() {
	using value_type = strsim::degree_generator::value_type;
	double seed = _dist(_gen);
	// search for seed over cdf using binary search
	value_type f = 0;
	value_type l = _size - 1;
	while (f < l) {
		value_type m = (f + l) / 2;
		if (_cdf[m] <= seed && seed < _cdf[m+1]) {
			return m + 1;
		}else if (_cdf[m] > seed) {
			l = m - 1;
		}else{
			f = m + 1;
		}
	}
	//DBG(f+1);
	return l + 1;
}

void strsim::rateless_coder::encode(unsigned int inum, unsigned int onum,
		std::vector<strsim::coded_block *> &b) {
	using value_type = strsim::rateless_block::value_type;
	bool finished = false;
	soliton_generator gen(inum);
	std::default_random_engine rden;
	std::uniform_int_distribution<unsigned int> dist(0, inum - 1);
	_num_blocks = inum;
	_raw_table.clear();
	for (unsigned int i = 0; i < inum; ++i) {
		_raw_table.push_back(false);
	}
	while (!finished) {
		b.clear(); // prepare for new blocks
		for (unsigned int i = 0; i < onum; ++i) {
			rateless_block * block = new rateless_block();
			value_type num_raw_blocks = gen.sample();
			for (unsigned int i = 0; i < num_raw_blocks; ++i) {
				value_type raw = dist(rden);
				while (_raw_table[raw]) {
					raw = (raw + 1) % inum;
				}
				block->raw_blocks.push_back(raw);
				_raw_table[raw] = true;
			}
			b.push_back(block);
			for (auto raw : block->raw_blocks) {
				_raw_table[raw] = false;
			}
		}
		// check again to make sure that we could  restore the raw data
		// from coded blocks
		this->restart();
		for (auto block : b) {
			if (this->decode(block) == 0) {
				finished = true;
				this->restart();
				break;
			}
		}
	}
}

unsigned int strsim::rateless_coder::decode(strsim::coded_block * b) {
	if (b->type() != this->type()) {
		DBG("The type of the coded is unknown");
	}
	std::list<value_type> block =
		(static_cast<rateless_block*>(b))->raw_blocks;
	if (block.size() > 1) {
		// check if we could reduce the regree of the block
		for (	auto encoded_block = block.begin();
				encoded_block != block.end(); ) {
			for (auto decoded_block : _raw_queue) {
				if (*encoded_block == decoded_block) {
					block.erase(encoded_block);
				}else{
					encoded_block++;
				}
			}
		}	
	}
	std::list<value_type> buffer;
	if (block.size() > 1) {
		// if the degree of the block is still greater than 1 then
		// we put it on waiting list
		_coding_queue.push_back(block);
	}else if (block.size() == 1 && !_raw_table[block.front()]) {
		// the block is fully decoded so we put it on raw list
		// and use it for decoding other block
		value_type raw = block.front();
		_raw_queue.push_back(raw);
		_raw_table[raw] = true;
		buffer.push_back(raw);
	}
	// used block that has just recovered for decoding other blocks
	while (!buffer.empty()) {
		value_type raw = buffer.front();
		buffer.pop_front();
		for (	auto decoded_block = _coding_queue.begin();
			   	decoded_block != _coding_queue.end(); ) {
			for (	auto encoded_block = decoded_block->begin();
					encoded_block != decoded_block->end(); ) {
				if (raw == *encoded_block) {
					decoded_block->erase(encoded_block);
				}else{
					encoded_block++;
				}
			}
			if (decoded_block->size() == 1 &&
					!_raw_table[decoded_block->front()]) {
				value_type b = decoded_block->front();
				_raw_queue.push_back(b);
				_raw_table[b] = true;
				buffer.push_back(b);
				_coding_queue.erase(decoded_block);
			}else{
				decoded_block++;
			}
		}
	}
	return _num_blocks - _raw_queue.size();
}


