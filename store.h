#ifndef STORE_H
#define STORE_H

#include "code.h"
#include <vector>

namespace strsim {

	/**
	 * @brief read block from storage devices
	 *
	 * @param blocks blocks to be read at the same time
	 */
	void read_data(std::vector<coded_block>& blocks);

}

#endif

