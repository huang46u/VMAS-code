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

#ifndef CGOGN_CORE_TYPES_MAPS_CELL_H_
#define CGOGN_CORE_TYPES_MAPS_CELL_H_

#include <cgogn/core/types/maps/dart.h>

#include <cgogn/core/utils/assert.h>
#include <cgogn/core/utils/numerics.h>

/**
 * \file cgogn/core/types/maps/cell.h
 * \brief Orbit and cell definitions used in cgogn.
 */

namespace cgogn
{

enum Orbit : uint32
{
	DART = 0,								  // 1VERTEX
	BETA1,									  // 1VERTEX
	PHI1,									  // 2FACE
	BETA0_BETA1 = PHI1,						  // 2FACE = 1CC
	PHI2,									  // 2EDGE
	BETA0_BETA2 = PHI2,						  // 2EDGE
	PHI21,									  // 2VERTEX
	BETA1_BETA2 = PHI21,					  // 2VERTEX
	PHI1_PHI2,								  // 3VOLUME
	BETA0_BETA1_BETA2 = PHI1_PHI2,			  // 3VOLUME = 2CC
	PHI1_PHI3,								  // 3FACE
	BETA0_BETA1_BETA3 = PHI1_PHI3,			  // 3FACE
	PHI2_PHI3,								  // 3EDGE
	BETA0_BETA2_BETA3 = PHI2_PHI3,			  // 3EDGE
	PHI21_PHI31,							  // 3VERTEX
	BETA1_BETA2_BETA3 = PHI21_PHI31,		  // 3VERTEX
	PHI1_PHI2_PHI3,							  // 3CC
	BETA0_BETA1_BETA2_BETA3 = PHI1_PHI2_PHI3, // 3CC
	BETA0,
	END_ORBIT
};

static const std::size_t NB_ORBITS = Orbit::END_ORBIT;

inline std::string orbit_name(Orbit orbit)
{
	switch (orbit)
	{
	case Orbit::DART:
		return "cgogn::Orbit::DART";
	case Orbit::BETA1:
		return "cgogn::Orbit::BETA1";
	case Orbit::PHI1:
		return "cgogn::Orbit::PHI1";
	case Orbit::PHI2:
		return "cgogn::Orbit::PHI2";
	case Orbit::PHI21:
		return "cgogn::Orbit::PHI21";
	case Orbit::PHI1_PHI2:
		return "cgogn::Orbit::PHI1_PHI2";
	case Orbit::PHI1_PHI3:
		return "cgogn::Orbit::PHI1_PHI3";
	case Orbit::PHI2_PHI3:
		return "cgogn::Orbit::PHI2_PHI3";
	case Orbit::PHI21_PHI31:
		return "cgogn::Orbit::PHI21_PHI31";
	case Orbit::PHI1_PHI2_PHI3:
		return "cgogn::Orbit::PHI1_PHI2_PHI3";
	case Orbit::BETA0:
		return "cgogn::Orbit::BETA0";
	default:
		cgogn_assert_not_reached("This orbit does not exist");
		return "UNKNOWN";
	}
	cgogn_assert_not_reached("This orbit does not exist");
#ifdef NDEBUG
	return "UNKNOWN"; // little trick to avoid warning on VS
#endif
}

/**
 * \brief Cellular typing
 * \tparam ORBIT The type of the orbit used to create the Cell
 */
template <Orbit ORBIT_>
struct Cell
{
	static const Orbit ORBIT = ORBIT_;
	using Self = Cell<ORBIT>;

	/**
	 * \brief the dart representing this cell
	 */
	Dart dart_;

	/**
	 * \brief Creates a new empty Cell as a nil dart.
	 */
	inline Cell() : dart_()
	{
	}

	/**
	 * \brief Creates a new Cell from a dart.
	 * \param[in] d a dart
	 */
	inline explicit Cell(Dart d) : dart_(d)
	{
	}

	/**
	 * \brief Creates a new Cell from an uint32 (index of the dart of the cell).
	 * \param[in] i index of the dart of the cell
	 */
	inline explicit Cell(uint32 i) : dart_(i)
	{
	}

	/**
	 * \brief Copy constructor.
	 * Creates a new Cell from an another one.
	 * \param[in] c a cell
	 */
	inline Cell(const Self& c) : dart_(c.dart_)
	{
	}

	/**
	 * \brief Tests the validity of the cell.
	 * \retval true if the cell is valid
	 * \retval false otherwise
	 */
	inline bool is_valid() const
	{
		return !dart_.is_nil();
	}

	/**
	 * \brief Assigns to the left hand side cell the value
	 * of the right hand side cell.
	 * \param[in] rhs the cell to assign
	 * \return The cell with the assigned value
	 */
	inline Self& operator=(Self rhs)
	{
		dart_ = rhs.dart_;
		return *this;
	}

	/**
	 * \brief Converts a cell to an uint32 (dart index, not the index of the cell in the map)
	 * (used in cell_cache parallel_foreach_cell in order to be able to use a std::vector<uint32> from managed buffers)
	 */
	inline operator uint32() const
	{
		return dart_.index_;
	}

	/**
	 * \brief Prints a cell to a stream.
	 * \param[out] out the stream to print on
	 * \param[in] rhs the cell to print
	 */
	inline friend std::ostream& operator<<(std::ostream& out, const Self& rhs)
	{
		return out << rhs.dart_;
	}

	/**
	 * \brief Reads a cell from a stream.
	 * \param[in] in the stream to read from
	 * \param[out] rhs the cell read
	 */
	inline friend std::istream& operator>>(std::istream& in, Self& rhs)
	{
		in >> rhs.dart_;
		return in;
	}
};

} // namespace cgogn

#endif // CGOGN_CORE_TYPES_MAPS_CELL_H_
