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

#ifndef CGOGN_CORE_TYPES_MAPS_GMAP_GMAP2_H_
#define CGOGN_CORE_TYPES_MAPS_GMAP_GMAP2_H_

#include <cgogn/core/types/maps/gmap/gmap1.h>

namespace cgogn
{

struct GMap2 : public GMap1
{
	static const uint8 dimension = 2;

	using Parent = GMap1;

	using Vertex = Cell<Orbit::BETA1_BETA2>;
	using HalfEdge = Cell<Orbit::DART>;
	using Edge = Cell<Orbit::BETA0_BETA2>;
	using Face = Cell<Orbit::BETA0_BETA1>;
	using Volume = Cell<Orbit::BETA0_BETA1_BETA2>;
	using CC = Volume;

	using BoundaryCell = Face;

	using Cells = std::tuple<Vertex, HalfEdge, Edge, Face, Volume>;

	std::shared_ptr<Attribute<Dart>> beta2_;

	GMap2() : GMap1()
	{
		beta2_ = add_relation("beta2");
	}
};

template <>
struct mesh_traits<GMap2>
{
	static constexpr const char* name = "GMap2";
	static constexpr const uint8 dimension = 2;

	using Parent = GMap2::Parent;

	using Vertex = GMap2::Vertex;
	using HalfEdge = GMap2::HalfEdge;
	using Edge = GMap2::Edge;
	using Face = GMap2::Face;
	using Volume = GMap2::Volume;

	using Cells = std::tuple<Vertex, HalfEdge, Edge, Face, Volume>;
	static constexpr const char* cell_names[] = {"Vertex", "HalfEdge", "Edge", "Face", "Volume"};

	template <typename T>
	using Attribute = MapBase::Attribute<T>;
	using AttributeGen = MapBase::AttributeGen;
	using MarkAttribute = MapBase::MarkAttribute;
};

/*************************************************************************/
// Basic beta functions
/*************************************************************************/

inline Dart beta2(const GMap2& m, Dart d)
{
	return (*(m.beta2_))[d.index_];
}

inline Dart boundary_beta1(const GMap2& m, Dart d)
{
	return beta2(m, d);
}

inline bool on_boundary(const GMap2& m, Dart d)
{
	return beta2(m, d) == d;
}

inline void beta2_sew(GMap2& m, Dart d, Dart e)
{
	cgogn_assert(beta2(m, d) == d);
	cgogn_assert(beta2(m, e) == e);
	(*(m.beta2_))[d.index_] = e;
	(*(m.beta2_))[e.index_] = d;
}

inline void beta2_unsew(GMap2& m, Dart d)
{
	Dart e = beta2(m, d);
	(*(m.beta2_))[d.index_] = d;
	(*(m.beta2_))[e.index_] = e;
}

inline Dart phi2(const GMap2& m, Dart d)
{
	Dart dd = beta2(m, d);
	if (dd == d)
		return d;
	return beta0(m, dd);
}

inline void phi2_sew(GMap2& m, Dart d, Dart e)
{
	Dart e0 = beta0(m, e);
	beta2_sew(m, d, e0);
	Dart d0 = beta0(m, d);
	beta2_sew(m, d0, e);
}

inline void phi2_unsew(GMap2& m, Dart d)
{
	Dart e = beta0(m, d);
	beta2_unsew(m, d);
	beta2_unsew(m, e);
}

/*************************************************************************/
// Operators
/*************************************************************************/

GMap2::Vertex cut_edge(GMap2& m, GMap2::Edge e, bool set_indices = true);
GMap2::Vertex collapse_edge(GMap2& m, GMap2::Edge e, bool set_indices = true);
bool flip_edge(GMap2& m, GMap2::Edge e, bool set_indices = true);

bool edge_can_collapse(const GMap2& m, GMap2::Edge e);
bool edge_can_flip(const GMap2& m, GMap2::Edge e);

GMap2::Face add_face(GMap2& m, uint32 size, bool set_indices = true);
void merge_incident_faces(GMap2& m, GMap2::Edge e, bool set_indices = true);
GMap2::Edge cut_face(GMap2& m, GMap2::Vertex v1, GMap2::Vertex v2, bool set_indices = true);

GMap2::Volume add_pyramid(GMap2& m, uint32 size, bool set_indices = true);
GMap2::Volume add_prism(GMap2& m, uint32 size, bool set_indices = true);
void remove_volume(GMap2& m, GMap2::Volume v);

GMap2::Face close_hole(GMap2& m, Dart d, bool set_indices);
int32 close(GMap2& m, bool set_indices = false);

inline void reverse_orientation(GMap2& m)
{
	// NOTHING TO DO
}

/*************************************************************************/
// Debugging helper functions
/*************************************************************************/

bool check_integrity(GMap2& m, bool verbose = true);

} // namespace cgogn

#endif // CGOGN_CORE_TYPES_MAPS_GMAP_GMAP2_H_
