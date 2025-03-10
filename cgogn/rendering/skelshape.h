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

#ifndef CGOGN_RENDERING_SKEL_SHAPE_DRAWER_H_
#define CGOGN_RENDERING_SKEL_SHAPE_DRAWER_H_

#include <cgogn/rendering/cgogn_rendering_export.h>
#include <cgogn/rendering/shader_program.h>
#include <cgogn/rendering/vbo_update.h>

#include <cgogn/geometry/types/vector_traits.h>
#include <Eigen/Dense>

namespace cgogn
{

namespace rendering
{
/*
ajouter:
void ShaderParam::set_vbos_inst(const std::vector<VBO*>& vbos)
void VBO::associate(GLuint attrib, int32 stride = 0, uint32 first = 0, uint32 divisor = 0 )
*/	
class ShaderParamSkelShape : public ShaderParam
{
public:

	//subdiv
	int nbs_;
	//color RBGA 
	GLColor color_;

	GLVec3 light_position_;

	std::unique_ptr<rendering::VAO> inst_vao_;

	ShaderParamSkelShape(ShaderProgram* prg);

	virtual void draw_inst(const GLMat4& projection, const GLMat4& view, uint32 nbi) = 0;

	inline void set_subdiv(int32 nb)
	{
		nbs_ = nb;
	}

	inline void set_light_pos(const GLVec3& lp)
	{
		light_position_ = lp;
	}

	inline void set_color(const GLColor& col)
	{
		color_ = col;
	}

};


DECLARE_SHADER_CLASS(SkelSphere, false, CGOGN_STR(SkelSphere))

class CGOGN_RENDERING_EXPORT ShaderParamSkelSphere : public ShaderParamSkelShape
{
	void set_uniforms() override;

public:

	using ShaderType = ShaderSkelSphere;

	inline ShaderParamSkelSphere(ShaderType* sh):
	ShaderParamSkelShape(sh) 
	{}

	inline ~ShaderParamSkelSphere() override {}

	void draw_inst(const GLMat4& projection, const GLMat4& view, uint32 nbi) override;
};

class SkelSphereDrawer
{
	VBO vbo_;
public:
	std::unique_ptr<ShaderSkelSphere::Param> param_;
	SkelSphereDrawer();
	void set_vertices_spheres(const std::vector<GLVec4>& P1);
	void set_subdiv(uint32 sub);
	void draw(const GLMat4& projection, const GLMat4& view);
};



DECLARE_SHADER_CLASS(SkelCone, false, CGOGN_STR(SkelCone))

class CGOGN_RENDERING_EXPORT ShaderParamSkelCone : public ShaderParamSkelShape
{

	void set_uniforms() override;

public:

	using ShaderType = ShaderSkelCone;

	inline ShaderParamSkelCone(ShaderType* sh):
	ShaderParamSkelShape(sh) 
	{}

	inline ~ShaderParamSkelCone() override {}

	void draw_inst(const GLMat4& projection, const GLMat4& view, uint32 nbi) override;
};

class SkelConeDrawer
{
	VBO vbo1_;
	VBO vbo2_;
	VBO vbo3_;
	VBO vbo4_;

	void set_real_cones(const std::vector<GLVec4>& P1, const std::vector<GLVec4>& P2, const std::vector<GLVec3>& N1,
						const std::vector<GLVec3>& N2);
	void compute_skel_cone(const GLVec4& A, const GLVec4& B, GLVec4& P1, GLVec4& P2, GLVec3& N1, GLVec3& N2);

public:
	std::unique_ptr<ShaderSkelCone::Param> param_;

	SkelConeDrawer();

	void set_edges_cones(const std::vector<GLVec4>& edges);

	void draw(const GLMat4& projection, const GLMat4& view);

};


DECLARE_SHADER_CLASS(SkelTri, false, CGOGN_STR(SkelTri))

class CGOGN_RENDERING_EXPORT ShaderParamSkelTri : public ShaderParamSkelShape
{
	void set_uniforms() override;
public:

	using ShaderType = ShaderSkelTri;

	inline ShaderParamSkelTri(ShaderType* sh):
	ShaderParamSkelShape(sh) 
	{}

	inline ~ShaderParamSkelTri() override {}

	void draw_inst(const GLMat4& projection, const GLMat4& view, uint32 nbi) override;
};

class SkelTriDrawer
{
	VBO vbo_p_;
	VBO vbo_n_;

	void compute_CDisk(const GLVec4& A, const GLVec4& B, GLVec3& P1, GLVec3& P2);
	void interPPS(const GLVec4& Pa, const GLVec4& Pb,
					const GLVec4& Sph, GLVec3& I1, GLVec3& I2);
	void compute_tri2(const GLVec4& P1, const GLVec4& P2, const GLVec4& P3, GLVec3* I);

public:
	std::unique_ptr<ShaderSkelTri::Param> param_;
	SkelTriDrawer();
	void set_tris_faces(const std::vector<GLVec4>& pts);
	void draw(const GLMat4& projection, const GLMat4& view);
};


/*
  must be created with correct OGL context !
  usage:
  1 fill the buffers with add_vertex add_edge and add tri
    (parameter can be any type of VEC which offer [] operator and good number of components.
  2 call update
  3 draw
*/
class SkelShapeDrawer
{
	std::vector<GLVec4> vertices_;
	std::vector<GLVec4> edges_;
	std::vector<GLVec4> triangles_;

	SkelSphereDrawer sphere_drawer_;
	SkelConeDrawer cone_drawer_;
	SkelTriDrawer tri_drawer_;

public:
	inline SkelShapeDrawer()
	{}


	template<typename VEC4>
	void add_vertex(const VEC4& v)
	{
		vertices_.emplace_back(float(v[0]), float(v[1]), float(v[2]), float(v[3]));
	}

	template <typename VEC3, typename SCAL>
	inline void add_vertex(const VEC3& v, SCAL r)
	{
		vertices_.emplace_back(float(v[0]), float(v[1]), float(v[2]), float(r));
	}

	template <typename VEC4_1, typename VEC4_2>
	void add_edge(const VEC4_1& e1, const VEC4_2& e2)
	{
		edges_.emplace_back(float(e1[0]), float(e1[1]), float(e1[2]), float(e1[3]));
		edges_.emplace_back(float(e2[0]), float(e2[1]), float(e2[2]), float(e2[3]));
	}

	template <typename VEC3_1, typename VEC3_2, typename SCAL_1, typename SCAL_2>
	void add_edge(const VEC3_1& e1, SCAL_1 r1, const VEC3_2& e2, SCAL_2 r2)
	{
		edges_.emplace_back(float(e1[0]), float(e1[1]), float(e1[2]), float(r1));
		edges_.emplace_back(float(e2[0]), float(e2[1]), float(e2[2]), float(r2));
	}

	template <typename VEC4_1, typename VEC4_2, typename VEC4_3>
	void add_triangle(const VEC4_1& t1, const VEC4_2& t2, const VEC4_3& t3)
	{
		triangles_.emplace_back(float(t1[0]), float(t1[1]), float(t1[2]), float(t1[3]));
		triangles_.emplace_back(float(t2[0]), float(t2[1]), float(t2[2]), float(t2[3]));
		triangles_.emplace_back(float(t3[0]), float(t3[1]), float(t3[2]), float(t3[3]));
	}

	template <typename VEC3_1, typename VEC3_2, typename VEC3_3, typename SCAL_1, typename SCAL_2, typename SCAL_3>
	void add_triangle(const VEC3_1& t1, SCAL_1 r1, const VEC3_2& t2, SCAL_2 r2, const VEC3_3& t3, SCAL_3 r3)
	{
		triangles_.emplace_back(float(t1[0]), float(t1[1]), float(t1[2]), float(r1));
		triangles_.emplace_back(float(t2[0]), float(t2[1]), float(t2[2]), float(r2));
		triangles_.emplace_back(float(t3[0]), float(t3[1]), float(t3[2]), float(r3));
	}

	/*
	 * eed to be called after filling the buffers (and before calling draw)
	 */
	inline void update()
	{
		sphere_drawer_.set_vertices_spheres(vertices_);
		cone_drawer_.set_edges_cones(edges_);
		tri_drawer_.set_tris_faces(triangles_);
	}

	inline void draw(const GLMat4& proj_matrix, const GLMat4& view_matrix)
	{
		sphere_drawer_.draw(proj_matrix, view_matrix);
		cone_drawer_.draw(proj_matrix, view_matrix);
		tri_drawer_.draw(proj_matrix, view_matrix);
	}

	inline void set_subdiv(uint32 nbs)
	{
		sphere_drawer_.set_subdiv(nbs);
		cone_drawer_.param_->set_subdiv(nbs);
	}

	inline void set_color(const GLColor col)
	{
		sphere_drawer_.param_->set_color(col);
		cone_drawer_.param_->set_color(col);
		tri_drawer_.param_->set_color(col);
	}
};



} // namespace rendering

} // namespace cgogn

#endif // CGOGN_RENDERING_SHAPE_DRAWER_H_
