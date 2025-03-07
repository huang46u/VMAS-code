/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * Copyright (C), IGG Group, ICube, University of Strasbourg, France            *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#ifndef CGOGN_CORE_UTILS_THREAD_H_
#define CGOGN_CORE_UTILS_THREAD_H_

#include <cgogn/core/cgogn_core_export.h>

#include <cgogn/core/utils/buffers.h>
#include <cgogn/core/utils/definitions.h>
#include <cgogn/core/utils/numerics.h>

#include <thread>

namespace cgogn
{

const uint32 PARALLEL_BUFFER_SIZE = 1024u;

// thread local values

extern CGOGN_TLS uint32 thread_index_;
extern CGOGN_TLS Buffers<uint32>* uint32_buffers_thread_;

extern CGOGN_TLS uint32 uint32_value_;
extern CGOGN_TLS float64 float64_value_;

CGOGN_CORE_EXPORT uint32& uint32_value();
CGOGN_CORE_EXPORT float64& float64_value();

/**
 * @brief function to call at the beginning of each thread which uses CGoGN
 * @param index index of the thread
 */
CGOGN_CORE_EXPORT void thread_start(uint32 index = 0);

/**
 * @brief function to call at end of each thread which use a map
 */
CGOGN_CORE_EXPORT void thread_stop();

CGOGN_CORE_EXPORT uint32 max_nb_threads();

CGOGN_CORE_EXPORT uint32 current_thread_index();
CGOGN_CORE_EXPORT uint32 current_worker_index();

CGOGN_CORE_EXPORT Buffers<uint32>* uint32_buffers();

template <typename F>
void launch_thread(F f)
{
	static bool launched = false;
	if (!launched)
	{
		std::thread t([f]() {
			launched = true;
			thread_start(max_nb_threads() - 1); // additional thread gets the max id
			f();
			launched = false;
		});
		t.detach();
	}
}

} // namespace cgogn

#endif // CGOGN_CORE_UTILS_THREAD_H_
