#ifndef CODE_H
#define CODE_H

#include <vector>
#include <string>
#include <random>

#define RATELESS_TYPE 1

namespace strsim {

	class coded_block {
	public:
		/** Block type. */
		int virtual type(void) = 0;
		unsigned long arrive_time;
		std::vector<int> raw_blocks; /// raw blocks contribute to this block
	};

	/** Encode raw data to a set of @ref coded_block and reverse */
	class coder {
	protected:
		std::vector<bool> _raw_blocks;
		unsigned int _block_left;
	public:

		coder() : _raw_blocks(), _block_left(0) {}

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
				std::vector<coded_block> &b) = 0;

		/**
		 * @brief reconstruct the original data by adding a new coded block
		 *
		 * @param[in] b coded block used for the decoding process
		 *
		 * @return number of raw blocks that has not been restored yet.
		 * If this nunber equal to 0 then the decoding
		 * process ends successfully.
		 */
		unsigned int virtual decode(coded_block b) = 0;
		
		/** restart the decoding process */
		void restart(void) {
			for (auto block : _raw_blocks) {
				block = false;
			}
			_block_left = _raw_blocks.size();
		}

		/** indicate if the decoding process ended or not */
		bool inline has_finish(void) {return _block_left == 0; }
	
		/** decoder type, blocks used for restoring the original data must
		 * have the same type. */
		int virtual type(void) = 0;

		/** give a raw block to decoder so it does have to restore
		 * this block from coded blocks */
		void virtual feed(int index) {
			if (_raw_blocks[index] == false) {
				_block_left--;
				_raw_blocks[index] = true;
			}
		}

	};

	class rateless_coder : coder {
	public:
		int type(void) { return RATELESS_TYPE; }
		void  encode(unsigned int inum, unsigned int onum,
				std::vector<coded_block> &b);
		unsigned int decode(coded_block b);
	};

	class degree_generator {
	public:
		typedef unsigned int value_type;
		/** Setup the generator, must be called before sampling */
		void virtual setup(value_type seed) = 0;
		/** Sampling from the generator */
		value_type virtual sample(void) = 0;
	};

	class soliton_generator : degree_generator {

	private:
		// use uniform distribution for random generation
		std::uniform_real_distribution<double> _dist;
		double * _cdf; // table for cdf
		value_type _size;

	public:
		soliton_generator() : _dist(0, 1), _cdf(nullptr) {};
		soliton_generator(value_type seed) : soliton_generator() {
			setup(seed);
		}
		~soliton_generator() { delete _cdf; };
		/** Setup the generator, must be called before sampling */
		void virtual setup(value_type seed);
		/** Sampling from the generator */
		value_type sample();
	};

}

#endif
