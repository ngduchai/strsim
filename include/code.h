#ifndef CODE_H
#define CODE_H

#include <vector>
#include <string>
#include <random>
#include <list>
#include "common.h"

#define RATELESS_TYPE 1

namespace strsim {

	class coded_block {
	public:
		/** Block type. */
		int virtual type(void) = 0;
		time_t arrieve_time;
		virtual ~coded_block() {};
	};

	class rateless_block : public coded_block {	
	public:
		typedef unsigned int value_type;
		typedef unsigned int size_type;
		std::list<value_type> raw_blocks;
		int inline virtual type(void) { return RATELESS_TYPE; }
		virtual ~rateless_block() {};
	};

	/** Encode raw data to a set of @ref coded_block and reverse */
	class coder {
	public:
		/**
		 * @brief generate coded blocks from raw data.
		 * The method also restarts the decode engine.
		 *
		 * @param[in] inum number of raw blocks
		 * @param[in] onum number of coded blocks
		 * @param[out] b a vector to hold the encoded blocks
		 *
		 */
		void virtual encode(unsigned int inum, unsigned int onum,
				std::vector<coded_block *> &b) = 0;

		/**
		 * @brief reconstruct the original data by adding a new coded block
		 *
		 * @param[in] b coded block used for the decoding process
		 *
		 * @return number of raw blocks that has not been restored yet.
		 * If this nunber equal to 0 then the decoding
		 * process ends successfully.
		 */
		unsigned int virtual decode(coded_block * b) = 0;
		
		/** restart the decoding process */
		void virtual restart(void) = 0;

		/** indicate if the decoding process ended or not */
		bool virtual has_finished(void) = 0;
	
		/** decoder type, blocks used for restoring the original data must
		 * have the same type. */
		int virtual type(void) = 0;

		/** give a raw block to decoder so it does not have to restore
		 * this block from coded blocks */
		void virtual feed(int /* index */) = 0;

	};

	class rateless_coder : public coder {
	public:
		typedef unsigned int value_type;
		typedef unsigned int size_type;
	private:
		std::list<value_type> _raw_queue;
		std::list<std::list<value_type>> _coding_queue;
		std::vector<value_type> _raw_table;
		size_type _num_blocks;
	public:
		int type(void) { return RATELESS_TYPE; }
		void encode(unsigned int inum, unsigned int onum,
				std::vector<coded_block *> &b);
		unsigned int decode(coded_block * b);
		
		void virtual restart(void) {
			for (size_type i = 0; i < _num_blocks; ++i) {
				_raw_table[i] = false;
			}
			_raw_queue.clear();
			_coding_queue.clear();
		}

		bool virtual inline has_finished(void) {
			return _num_blocks == _raw_queue.size();
		}

		void virtual feed(int index) {
			if (!_raw_table[index]) {
				_raw_queue.push_back(index);
				_raw_table[index] = true;
			}
		};
	};

	
	class soliton_generator : public rnd_generator {
	private:
		// use uniform distribution for random generation
		std::mt19937 _gen;
		std::uniform_real_distribution<double> _dist;
		double * _cdf; // table for cdf
		value_type _size;
	public:
		soliton_generator() : _gen(std::random_device()()),
				_dist(0, 1), _cdf(nullptr) {};
		soliton_generator(value_type seed) : soliton_generator() {
			setup(seed);
		}
		~soliton_generator() { delete _cdf; };
		/** Setup the generator, must be called before sampling */
		void setup(value_type seed);
		/** Sampling from the generator */
		value_type sample();
	};

}

#endif
