/*******************************************************************************
 * CGoGN                                                                        *
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

#ifndef CGOGN_MODULE_VMAS_H_
#define CGOGN_MODULE_VMAS_H_

#include <cgogn/ui/app.h>
#include <cgogn/ui/module.h>

#include <cgogn/core/ui_modules/mesh_provider.h>

#include <cgogn/geometry/algos/area.h>
#include <cgogn/geometry/algos/fitting.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/medial_axis.h>
#include <cgogn/geometry/functions/angle.h>
#include <cgogn/geometry/functions/distance.h>
#include <cgogn/geometry/types/spherical_quadric.h>

#include <Eigen/Sparse>
#include <libacc/bvh_tree.h>
#include <libacc/bvh_tree_spheres.h>
#include <libacc/kd_tree.h>

#include <GLFW/glfw3.h>

#include <boost/synapse/connect.hpp>
#include <set>

namespace cgogn
{

namespace ui
{

using geometry::Mat3;
using geometry::Mat4;
using geometry::Scalar;
using geometry::Spherical_Quadric;
using geometry::Vec3;
using geometry::Vec4;

template <typename SURFACE, typename POINTS, typename NONMANIFOLD> class VMAS : public ViewModule
{
    template <typename T> using SAttribute = typename mesh_traits<SURFACE>::template Attribute<T>;
    using SVertex = typename mesh_traits<SURFACE>::Vertex;
    using SEdge = typename mesh_traits<SURFACE>::Edge;
    using SFace = typename mesh_traits<SURFACE>::Face;

    template <typename T> using PAttribute = typename mesh_traits<POINTS>::template Attribute<T>;
    using PVertex = typename mesh_traits<POINTS>::Vertex;

    template <typename T> using NMAttribute = typename mesh_traits<NONMANIFOLD>::template Attribute<T>;
    using NMVertex = typename mesh_traits<NONMANIFOLD>::Vertex;
    using NMEdge = typename mesh_traits<NONMANIFOLD>::Edge;

    enum AutoSplitMode : uint32
    {
        MAX_NB_SPHERES,
        ERROR_THRESHOLD
    };

    enum CorrectionMode : uint32
    {
        CORRECT_ALWAYS,
        CORRECT_ON_SPLIT
    };

	const uint32 k = 6; //kNN parameter

    struct SurfaceParameters
    {
        bool initialized_ = false;

        SURFACE *surface_;
        std::shared_ptr<SAttribute<Vec3>> surface_vertex_position_ = nullptr;
        std::shared_ptr<SAttribute<Vec3>> surface_vertex_position_original_ = nullptr;
        std::shared_ptr<SAttribute<Vec3>> surface_vertex_normal_ = nullptr;
        std::shared_ptr<SAttribute<Vec3>> surface_vertex_color_ = nullptr;
        std::shared_ptr<SAttribute<Vec3>> surface_face_normal_ = nullptr;
        std::shared_ptr<SAttribute<Scalar>> surface_vertex_area_pc_ = nullptr;
        std::shared_ptr<SAttribute<Scalar>> surface_vertex_area_surf_ = nullptr;
        std::shared_ptr<SAttribute<Scalar>> surface_face_area_ = nullptr;
        std::shared_ptr<SAttribute<std::vector<SVertex>>> surface_vertex_knn_ = nullptr;
        std::shared_ptr<SAttribute<Spherical_Quadric>> surface_vertex_quadric_ = nullptr;
        std::shared_ptr<SAttribute<Vec3>> medial_axis_position_ = nullptr;
        std::shared_ptr<SAttribute<Scalar>> medial_axis_radius_ = nullptr;
        std::shared_ptr<SAttribute<SVertex>> medial_axis_secondary_vertex_ = nullptr;
        std::shared_ptr<SAttribute<PVertex>> surface_vertex_sphere_ = nullptr;
        std::shared_ptr<SAttribute<Scalar>> surface_vertex_error_ = nullptr;

        acc::BVHTree<uint32, Vec3> *surface_bvh_ = nullptr;
        std::vector<SFace> surface_bvh_faces_;
        acc::KDTree<3, uint32> *surface_kdt_ = nullptr;
        std::vector<SVertex> surface_kdt_vertices_;

        float32 noise_factor_ = 0.01f;

        bool point_cloud_mode_ = false;

        float32 sqem_update_lambda_ = 0.2f;
        float32 sqem_clustering_lambda_ = 0.2f; // initialized with mean edge length

        POINTS *spheres_;
        uint32 nb_spheres_ = 0;
        std::shared_ptr<PAttribute<Vec3>> spheres_position_ = nullptr;
        std::shared_ptr<PAttribute<Scalar>> spheres_radius_ = nullptr;
        std::shared_ptr<PAttribute<Vec4>> spheres_color_ = nullptr;
        std::shared_ptr<PAttribute<std::vector<SVertex>>> spheres_cluster_ = nullptr;
        std::shared_ptr<PAttribute<Scalar>> spheres_cluster_area_ = nullptr;
        std::shared_ptr<PAttribute<Vec3>> spheres_cluster_color_ = nullptr;
        std::shared_ptr<PAttribute<std::set<PVertex>>> spheres_neighbor_clusters_ = nullptr;
        std::shared_ptr<PAttribute<bool>> spheres_do_not_split_ = nullptr;
        std::shared_ptr<PAttribute<Scalar>> spheres_error_ = nullptr;
        std::shared_ptr<PAttribute<Scalar>> spheres_error_not_normalized_ = nullptr;
        // PAttribute<Scalar>* selected_spheres_error_ = nullptr;

        NONMANIFOLD *skeleton_;
        std::shared_ptr<NMAttribute<Vec3>> skeleton_position_ = nullptr;

        float32 filter_radius_threshold_ = 0.0f;
        float32 filter_angle_threshold_ = 0.0f;

        float32 init_dilation_factor_ = 3.0f;

        bool use_sqem_term_ = true;

        bool sphere_correction_ = true;
        CorrectionMode sphere_correction_mode_ = CORRECT_ALWAYS;

        bool auto_stop_ = false;
        bool auto_split_ = true;
        AutoSplitMode auto_split_mode_ = MAX_NB_SPHERES;
        float32 auto_split_error_threshold_ = 0.00025f;
        uint32 auto_split_max_nb_spheres_ = 50;
		uint32 init_max_nb_spheres_ = 1;

        Scalar total_error_ = 0.0;
        Scalar total_error_not_normalized_ = 0.0;
        Scalar last_total_error_ = 0.0;
        Scalar total_error_diff_ = 0.0;
        Scalar min_error_ = 0.0;
        Scalar max_error_ = 0.0;
        PVertex max_error_sphere_ = PVertex();

        bool error_as_spheres_color_ = false;
        float32 spheres_transparency_ = 0.5f;

        uint32 iteration_count_ = 0;
        std::mutex mutex_;
        bool running_ = false;
        bool stopping_ = false;
        bool slow_down_ = true;
        uint32 update_rate_ =1000;
    };

  public:
   VMAS(const App &app)
        : ViewModule(app, "VMAS (" + std::string{mesh_traits<SURFACE>::name} + "," +
                              std::string{mesh_traits<POINTS>::name} + ")")
    {
    }
    ~VMAS()
    {
    }

    void set_selected_surface(SURFACE &s)
    {
        selected_surface_ = &s;
        picked_sphere_ = PVertex();
    }

    void set_surface_vertex_position(SURFACE &s, const std::shared_ptr<SAttribute<Vec3>> &surface_vertex_position)
    {
        SurfaceParameters &p = surface_parameters_[&s];
        p.surface_vertex_position_ = surface_vertex_position;
    }

    void add_surface_noise(SurfaceParameters &p)
    {
        if (!p.surface_vertex_position_ || !p.surface_vertex_normal_)
        {
            std::cout << "No surface vertex position or normal attribute set" << std::endl;
            return;
        }

        const MeshData<SURFACE> &md = surface_provider_->mesh_data(*p.surface_);
        Scalar bb_diag = (md.bb_max_ - md.bb_min_).norm() / 2.0;

        parallel_foreach_cell(*p.surface_, [&](SVertex v) -> bool {
            uint32 v_index = index_of(*p.surface_, v);
            float32 r = float32(rand()) / float32(RAND_MAX);
            int i = rand() % 2;
            if (i == 0)
                r *= -1.0f;
            (*p.surface_vertex_position_)[v_index] +=
                r * p.noise_factor_ * bb_diag * (*p.surface_vertex_normal_)[v_index];
            return true;
        });

        surface_provider_->emit_attribute_changed(*p.surface_, p.surface_vertex_position_.get());
    }

    void restore_surface_position(SurfaceParameters &p)
    {
        if (!p.surface_vertex_position_ || !p.surface_vertex_position_original_)
        {
            std::cout << "No surface vertex position or original position attribute set" << std::endl;
            return;
        }

        p.surface_vertex_position_->copy(p.surface_vertex_position_original_.get());

        surface_provider_->emit_attribute_changed(*p.surface_, p.surface_vertex_position_.get());
    }

    void compute_quadrics(SurfaceParameters &p)
    {
        parallel_foreach_cell(*p.surface_, [&](SVertex v) -> bool {
            uint32 v_index = index_of(*p.surface_, v);
            Spherical_Quadric &q = (*p.surface_vertex_quadric_)[v_index];
            q.clear();
            const Vec3 &pos = (*p.surface_vertex_position_)[v_index];
            if (p.point_cloud_mode_)
            {
                const Vec3 &n = (*p.surface_vertex_normal_)[v_index];
                Scalar a = (*p.surface_vertex_area_pc_)[v_index] / (k + 1.0);
                q += Spherical_Quadric(Vec4(pos.x(), pos.y(), pos.z(), 0), Vec4(n.x(), n.y(), n.z(), 1)) * a;
                for (SVertex vn : (*p.surface_vertex_knn_)[v_index])
                {
                    uint32 vn_index = index_of(*p.surface_, vn);
                    const Vec3 &pn = (*p.surface_vertex_position_)[vn_index];
                    const Vec3 &nn = (*p.surface_vertex_normal_)[vn_index];
                    Scalar an = (*p.surface_vertex_area_pc_)[vn_index] / (k + 1.0);
                    q += Spherical_Quadric(Vec4(pn.x(), pn.y(), pn.z(), 0), Vec4(nn.x(), nn.y(), nn.z(), 1)) * an;
                }
            }
            else
            {
                foreach_incident_face(*p.surface_, v, [&](SFace f) -> bool {
                    const Vec3 &n = value<Vec3>(*p.surface_, p.surface_face_normal_, f);
                    Scalar a = value<Scalar>(*p.surface_, p.surface_face_area_, f) / 3.0;
                    q += Spherical_Quadric(Vec4(pos.x(), pos.y(), pos.z(), 0), Vec4(n.x(), n.y(), n.z(), 1)) * a;
                    return true;
                });
            }
            return true;
        });
    }

    void init_surface_data(SURFACE &s)
    {
        SurfaceParameters &p = surface_parameters_[&s];
        p.surface_ = &s;

        if (!p.surface_vertex_position_)
        {
            std::cout << "No surface vertex position attribute set" << std::endl;
            return;
        }

        // set signal connections to update the data when the surface connectivity or position changes

        if (surface_connections_.find(&s) == surface_connections_.end())
        {
            surface_connections_[&s].push_back(
                boost::synapse::connect<typename MeshProvider<SURFACE>::connectivity_changed>(&s, [this, &s = s]() {
                    SurfaceParameters &p = surface_parameters_[&s];
                    init_surface_data(s);
                }));
            surface_connections_[&s].push_back(
                boost::synapse::connect<typename MeshProvider<SURFACE>::template attribute_changed_t<Vec3>>(
                    &s, [this, &s = s](SAttribute<Vec3> *attribute) {
                        SurfaceParameters &p = surface_parameters_[&s];
                        if (attribute == p.surface_vertex_position_.get())
                            init_surface_data(s);
                    }));
        }

        // save original positions

        p.surface_vertex_position_original_ = get_attribute<Vec3, SVertex>(s, "position_original");
        if (!p.surface_vertex_position_original_)
        {
            p.surface_vertex_position_original_ = add_attribute<Vec3, SVertex>(s, "position_original");
            p.surface_vertex_position_original_->copy(p.surface_vertex_position_.get());
        }

        // create BVH and KDTree for the surface

        MeshData<SURFACE> &md = surface_provider_->mesh_data(s);
        uint32 nb_vertices = md.template nb_cells<SVertex>();
        uint32 nb_faces = md.template nb_cells<SFace>();

        auto bvh_vertex_index = get_or_add_attribute<uint32, SVertex>(s, "__bvh_vertex_index");

        p.surface_kdt_vertices_.clear();
        p.surface_kdt_vertices_.reserve(nb_vertices);
        std::vector<Vec3> vertex_position_vector;
        vertex_position_vector.reserve(nb_vertices);
        uint32 idx = 0;
        foreach_cell(s, [&](SVertex v) -> bool {
            p.surface_kdt_vertices_.push_back(v);
            value<uint32>(s, bvh_vertex_index, v) = idx++;
            vertex_position_vector.push_back(value<Vec3>(s, p.surface_vertex_position_, v));
            return true;
        });

        p.surface_bvh_faces_.clear();
        p.surface_bvh_faces_.reserve(nb_faces);
        std::vector<uint32> face_vertex_indices;
        face_vertex_indices.reserve(nb_faces * 3);
        foreach_cell(s, [&](SFace f) -> bool {
            p.surface_bvh_faces_.push_back(f);
            foreach_incident_vertex(s, f, [&](SVertex v) -> bool {
                face_vertex_indices.push_back(value<uint32>(s, bvh_vertex_index, v));
                return true;
            });
            return true;
        });

        if (p.surface_bvh_)
            delete p.surface_bvh_;
        p.surface_bvh_ = new acc::BVHTree<uint32, Vec3>(face_vertex_indices, vertex_position_vector);

        if (p.surface_kdt_)
            delete p.surface_kdt_;
        p.surface_kdt_ = new acc::KDTree<3, uint32>(vertex_position_vector);

        remove_attribute<SVertex>(s, bvh_vertex_index);

        // compute knn graph

        p.surface_vertex_knn_ = get_or_add_attribute<std::vector<SVertex>, SVertex>(s, "knn");
        foreach_cell(s, [&](SVertex v) -> bool {
            uint32 v_index = index_of(*p.surface_, v);
            const Vec3 &pos = (*p.surface_vertex_position_)[v_index];
            std::vector<std::pair<uint32, Scalar>> k_res;
            p.surface_kdt_->find_nns(pos, k, &k_res);
            (*p.surface_vertex_knn_)[v_index].clear();
            (*p.surface_vertex_knn_)[v_index].reserve(k_res.size());
            for (const auto &[idx, dist] : k_res)
            {
                SVertex nv = p.surface_kdt_vertices_[idx];
                if (nv != v)
                    (*p.surface_vertex_knn_)[v_index].push_back(nv);
            }
            return true;
        });

        // compute normals

        p.surface_face_normal_ = get_or_add_attribute<Vec3, SFace>(s, "normal");
        geometry::compute_normal<SFace>(s, p.surface_vertex_position_.get(), p.surface_face_normal_.get());

        p.surface_vertex_normal_ = get_or_add_attribute<Vec3, SVertex>(s, "normal");
        geometry::compute_normal<SVertex>(s, p.surface_vertex_position_.get(), p.surface_vertex_normal_.get());

        // vertex colors

        p.surface_vertex_color_ = get_or_add_attribute<Vec3, SVertex>(s, "color");

        // compute areas

        p.surface_face_area_ = get_or_add_attribute<Scalar, SFace>(s, "area");
        geometry::compute_area<SFace>(s, p.surface_vertex_position_.get(), p.surface_face_area_.get());

        p.surface_vertex_area_pc_ = get_or_add_attribute<Scalar, SVertex>(s, "area");
        foreach_cell(s, [&](SVertex v) -> bool {
            uint32 v_index = index_of(s, v);
            Scalar sum = 0.0;
            const Vec3 &pos = (*p.surface_vertex_position_)[v_index];
            for (SVertex u : (*p.surface_vertex_knn_)[v_index])
                sum += (value<Vec3>(s, p.surface_vertex_position_, u) - pos).norm();
            (*p.surface_vertex_area_pc_)[v_index] = (sum * sum) / (2.0 * k);
            return true;
        });

        p.surface_vertex_area_surf_ = get_or_add_attribute<Scalar, SVertex>(s, "area");
        geometry::compute_area<SVertex>(s, p.surface_vertex_position_.get(), p.surface_vertex_area_surf_.get(),
                                        geometry::VertexAreaPolicy::THIRD);

        // estimate lambda for SQEM

        // Scalar mean_edge_length = geometry::mean_edge_length(s, p.surface_vertex_position_.get());
        // p.sqem_clustering_lambda_ = mean_edge_length * mean_edge_length * k;

        // initialize SQEM quadrics

        p.surface_vertex_quadric_ = get_or_add_attribute<Spherical_Quadric, SVertex>(s, "quadric");
        compute_quadrics(p);

        // compute shrinking balls for the surface vertices

        p.medial_axis_position_ = get_or_add_attribute<Vec3, SVertex>(s, "medial_axis_position");
        p.medial_axis_radius_ = get_or_add_attribute<Scalar, SVertex>(s, "medial_axis_radius");
        p.medial_axis_secondary_vertex_ = get_or_add_attribute<SVertex, SVertex>(s, "medial_axis_secondary_vertex_");

        parallel_foreach_cell(s, [&](SVertex v) -> bool {
            uint32 v_index = index_of(s, v);
            auto [c, r, q] = geometry::shrinking_ball_center(
                s, (*p.surface_vertex_position_)[v_index], (*p.surface_vertex_normal_)[v_index],
                p.surface_vertex_position_.get(), p.surface_bvh_, p.surface_bvh_faces_, p.surface_kdt_,
                p.surface_kdt_vertices_);
            (*p.medial_axis_position_)[v_index] = c;
            (*p.medial_axis_radius_)[v_index] = r;
            (*p.medial_axis_secondary_vertex_)[v_index] = q;
            return true;
        });

        // create the spheres mesh

        if (!p.spheres_)
            p.spheres_ = points_provider_->add_mesh(surface_provider_->mesh_name(s) + "_spheres");

        p.spheres_position_ = get_or_add_attribute<Vec3, PVertex>(*p.spheres_, "position");
        p.spheres_radius_ = get_or_add_attribute<Scalar, PVertex>(*p.spheres_, "radius");
        p.spheres_color_ = get_or_add_attribute<Vec4, PVertex>(*p.spheres_, "color");

        p.spheres_cluster_ = get_or_add_attribute<std::vector<SVertex>, PVertex>(
            *p.spheres_, "cluster"); // surface vertices in the cluster
        p.spheres_cluster_color_ = get_or_add_attribute<Vec3, PVertex>(*p.spheres_, "cluster_color");
        p.spheres_cluster_area_ = get_or_add_attribute<Scalar, PVertex>(*p.spheres_, "cluster_area");

        p.spheres_error_ = get_or_add_attribute<Scalar, PVertex>(*p.spheres_, "error");
        p.spheres_error_not_normalized_ = get_or_add_attribute<Scalar, PVertex>(*p.spheres_, "error_not_normalized");
        p.spheres_error_->fill(0.0);
        p.spheres_error_not_normalized_->fill(0.0);

        p.spheres_do_not_split_ = get_or_add_attribute<bool, PVertex>(*p.spheres_, "do_not_split");

        p.surface_vertex_error_ =
            get_or_add_attribute<Scalar, SVertex>(s, "error"); // error of a vertex w.r.t. its sphere

        p.surface_vertex_sphere_ = get_or_add_attribute<PVertex, SVertex>(s, "sphere"); // cluster of the surface vertex
        p.spheres_neighbor_clusters_ =
            get_or_add_attribute<std::set<PVertex>, PVertex>(*p.spheres_, "neighbor_clusters"); // neighbor clusters

        // create the skeleton mesh

        if (!p.skeleton_)
            p.skeleton_ = non_manifold_provider_->add_mesh(remove_extension(surface_provider_->mesh_name(s)) + "_skeleton");
        p.skeleton_position_ = get_or_add_attribute<Vec3, NMVertex>(*p.skeleton_, "position");

        // if we already have spheres, we need to recompute the clusters and errors
        // (cleans out surface vertex sphere data)

        compute_clusters(p);

        // update the render data (spheres and skeleton)
        // (the skeleton is reconstructed)

        if (!p.running_)
            update_render_data(p);

        p.initialized_ = true;
    }

    // void set_selected_spheres_error(SurfaceParameters& p, PAttribute<Scalar>* attribute)
    // {
    // 	p.selected_spheres_error_ = attribute;
    // 	update_spheres_color(p);
    // 	if (!p.running_)
    // 		update_render_data(p);
    // }

    // uint32 nb_covered_vertices(SurfaceParameters& p, SVertex v)
    // {
    // 	uint32 v_index = index_of(*p.surface_, v);
    // 	const Vec3& vp = (*p.medial_axis_position_)[v_index];
    // 	Scalar vr = (*p.medial_axis_radius_)[v_index];
    // 	uint32 count = 0;
    // 	foreach_cell(*p.surface_, [&](SVertex w) -> bool {
    // 		if ((value<Vec3>(*p.surface_, p.surface_vertex_position_, w) - vp).norm() < p.init_dilation_factor_ * vr)
    // 			++count;
    // 		return true;
    // 	});
    // 	return count;
    // }

    void init_spheres(SURFACE &s, uint32 max_nb_spheres)
    {
        SurfaceParameters &p = surface_parameters_[&s];

        clear(*p.spheres_);

        // auto nb_covered = add_attribute<uint32, SVertex>(s, "__nb_covered");
        // parallel_foreach_cell(s, [&](SVertex v) -> bool {
        // 	value<uint32>(s, nb_covered, v) = nb_covered_vertices(s, v);
        // 	return true;
        // });

        std::vector<SVertex> sorted_vertices;
        foreach_cell(*p.surface_, [&](SVertex v) -> bool {
            sorted_vertices.push_back(v);
            return true;
        });
        std::sort(sorted_vertices.begin(), sorted_vertices.end(), [&](SVertex a, SVertex b) {
            // sort candidate spheres by decreasing radius
            return value<Scalar>(*p.surface_, p.medial_axis_radius_, a) >
                   value<Scalar>(*p.surface_, p.medial_axis_radius_, b);
            // sort candidate spheres by decreasing number of covered vertices
            // return value<uint32>(*p.surface_, nb_covered, a) > value<uint32>(*p.surface_, nb_covered, b);
        });

        // remove_attribute<SVertex>(*p.surface_, nb_covered);

        auto covered = add_attribute<bool, SVertex>(*p.surface_, "__covered");
        covered->fill(false);

        p.nb_spheres_ = 0;

        for (SVertex v : sorted_vertices)
        {
            uint32 v_index = index_of(*p.surface_, v);

            if (p.nb_spheres_ >= max_nb_spheres)
                break;

            // do not add spheres with radius smaller than filter_radius_threshold_
            if ((*p.medial_axis_radius_)[v_index] < p.filter_radius_threshold_)
                break;

            if ((*covered)[v_index])
                continue;

            const Vec3 &vp = (*p.medial_axis_position_)[v_index];
            Scalar vr = (*p.medial_axis_radius_)[v_index];

            PVertex sphere = add_vertex(*p.spheres_);
            p.nb_spheres_++;
            uint32 sphere_index = index_of(*p.spheres_, sphere);

            (*p.spheres_position_)[sphere_index] = vp;
            (*p.spheres_radius_)[sphere_index] = vr;
            (*p.spheres_cluster_color_)[sphere_index] =
                Vec3(0.5 + 0.5 * (rand() % 256) / 256.0, 0.5 + 0.5 * (rand() % 256) / 256.0,
                     0.5 + 0.5 * (rand() % 256) / 256.0);
            // Vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f);

            std::stack<SVertex> stack;
            stack.push(v);
            while (!stack.empty())
            {
                SVertex w = stack.top();
                stack.pop();
                value<bool>(*p.surface_, covered, w) = true;
                foreach_adjacent_vertex_through_edge(*p.surface_, w, [&](SVertex u) -> bool {
                    uint32 u_index = index_of(*p.surface_, u);
                    if (!(*covered)[u_index] &&
                        ((*p.surface_vertex_position_)[u_index] - vp).norm() < p.init_dilation_factor_ * vr)
                        stack.push(u);
                    return true;
                });
            }
            stack.push((*p.medial_axis_secondary_vertex_)[v_index]);
            while (!stack.empty())
            {
                SVertex w = stack.top();
                stack.pop();
                value<bool>(*p.surface_, covered, w) = true;
                foreach_adjacent_vertex_through_edge(*p.surface_, w, [&](SVertex u) -> bool {
                    uint32 u_index = index_of(*p.surface_, u);
                    if (!(*covered)[u_index] &&
                        ((*p.surface_vertex_position_)[u_index] - vp).norm() < p.init_dilation_factor_ * vr)
                        stack.push(u);
                    return true;
                });
            }
        }

        remove_attribute<SVertex>(*p.surface_, covered);

        compute_clusters(p);

        if (!p.running_)
            update_render_data(p);
    }

    void compute_clusters(SurfaceParameters &p)
    {
        // clean cluster affectation
        parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
            uint32 v_index = index_of(*p.spheres_, v);
            (*p.spheres_cluster_)[v_index].clear();
            (*p.spheres_cluster_area_)[v_index] = 0.0;
            return true;
        });
        p.surface_vertex_sphere_->fill(PVertex());

        MeshData<POINTS> &md = points_provider_->mesh_data(*p.spheres_);
        if (p.nb_spheres_ == 0)
            return;

        parallel_foreach_cell(*p.surface_, [&](SVertex v) -> bool {
            uint32 v_index = index_of(*p.surface_, v);

            Scalar a = 1.0;
            if (p.point_cloud_mode_)
                a = (*p.surface_vertex_area_pc_)[v_index];
            else
                a = (*p.surface_vertex_area_surf_)[v_index];

            const Vec3 &vp = (*p.surface_vertex_position_)[v_index];
            Scalar min_distance = std::numeric_limits<Scalar>::max();
            PVertex closest_sphere;
            uint32 closest_sphere_index;

            foreach_cell(*p.spheres_, [&](PVertex pv) {
                uint32 pv_index = index_of(*p.spheres_, pv);

                const Vec3 &center = (*p.spheres_position_)[pv_index];
                Scalar radius = (*p.spheres_radius_)[pv_index];
                Scalar dist_eucl = ((vp - center).norm() - radius);
                dist_eucl *= dist_eucl;
                dist_eucl *= a;
                Scalar dist;
                if (p.use_sqem_term_)
                {
                    Scalar dist_sqem =
                        (*p.surface_vertex_quadric_)[v_index].eval(Vec4(center.x(), center.y(), center.z(), radius));
                    dist = dist_sqem + p.sqem_clustering_lambda_ * dist_eucl;
                }
                else
                    dist = dist_eucl;
                if (dist < min_distance)
                {
                    min_distance = dist;
                    closest_sphere = pv;
                    closest_sphere_index = pv_index;
                }
                return true;
            });

            value<PVertex>(*p.surface_, p.surface_vertex_sphere_, v) = closest_sphere;

            std::lock_guard<std::mutex> lock(spheres_mutex_[closest_sphere_index % spheres_mutex_.size()]);
            value<std::vector<SVertex>>(*p.spheres_, p.spheres_cluster_, closest_sphere).push_back(v);
            value<Scalar>(*p.spheres_, p.spheres_cluster_area_, closest_sphere) += a;

            return true;
        });

        // remove small clusters
        foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
            std::vector<SVertex> &cluster = value<std::vector<SVertex>>(*p.spheres_, p.spheres_cluster_, v);
            if (cluster.size() < 4)
            {
                for (SVertex sv : cluster)
                    value<PVertex>(*p.surface_, p.surface_vertex_sphere_, sv) = PVertex();
                remove_vertex(*p.spheres_, v);
                p.nb_spheres_--;
            }
            return true;
        });
    }

    void compute_spheres_error(SurfaceParameters &p)
    {
        parallel_foreach_cell(*p.spheres_, [&](PVertex v) {
            uint32 v_index = index_of(*p.spheres_, v);

            const Vec3 &center = (*p.spheres_position_)[v_index];
            Scalar radius = (*p.spheres_radius_)[v_index];
            const std::vector<SVertex> &cluster = (*p.spheres_cluster_)[v_index];

            Scalar cluster_error = 0.0;
            for (SVertex sv : cluster)
            {
                uint32 sv_index = index_of(*p.surface_, sv);

                Scalar a = 1.0;
                if (p.point_cloud_mode_)
                    a = (*p.surface_vertex_area_pc_)[sv_index];
                else
                    a = (*p.surface_vertex_area_surf_)[sv_index];

                Scalar dist_eucl = ((*p.surface_vertex_position_)[sv_index] - center).norm() - radius;
                dist_eucl *= dist_eucl;
                dist_eucl *= a;
                Scalar dist;
                if (p.use_sqem_term_)
                {
                    Scalar dist_sqem =
                        (*p.surface_vertex_quadric_)[sv_index].eval(Vec4(center.x(), center.y(), center.z(), radius));
                    dist = dist_sqem + p.sqem_clustering_lambda_ * dist_eucl;
                }
                else
                    dist = dist_eucl;
                (*p.surface_vertex_error_)[sv_index] = dist;
                cluster_error += dist;
            }

            (*p.spheres_error_)[v_index] = cluster_error / (*p.spheres_cluster_area_)[v_index];
            (*p.spheres_error_not_normalized_)[v_index] = cluster_error;

            return true;
        });

        p.min_error_ = std::numeric_limits<Scalar>::max();
        p.max_error_ = std::numeric_limits<Scalar>::min();
        p.max_error_sphere_ = PVertex();
        p.total_error_ = 0.0;
        p.total_error_not_normalized_ = 0.0;

        foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
            Scalar error = value<Scalar>(*p.spheres_, p.spheres_error_, v);
            Scalar error_not_normalized = value<Scalar>(*p.spheres_, p.spheres_error_not_normalized_, v);
            p.min_error_ = std::min(p.min_error_, error);
            if (error > p.max_error_)
            {
                p.max_error_ = error;
                p.max_error_sphere_ = v;
            }
            p.total_error_ += error;
            p.total_error_not_normalized_ += error_not_normalized;
            return true;
        });

        p.total_error_diff_ = fabs(p.total_error_ - p.last_total_error_);
        p.last_total_error_ = p.total_error_;
    }

    void update_spheres_color(SurfaceParameters &p)
    {
        parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
            uint32 v_index = index_of(*p.spheres_, v);
            if (p.error_as_spheres_color_)
                (*p.spheres_color_)[v_index] =
                    color_map((*p.spheres_error_)[v_index], p.min_error_, p.max_error_, p.spheres_transparency_);
            else
            {
                const Vec3 &c = (*p.spheres_cluster_color_)[v_index];
                (*p.spheres_color_)[v_index] = Vec4(c.x(), c.y(), c.z(), p.spheres_transparency_);
            }
            return true;
        });
    }

    Vec4 color_map(Scalar x, Scalar min, Scalar max, float32 transparency = 1.0)
    {
        x = (x - min) / (max - min);
        x = std::clamp(x, 0.0, 1.0);

        Scalar x2 = 2.0 * x;
        switch (int(std::floor(std::max(0.0, x2 + 1.0))))
        {
        case 0:
            return Vec4(0.0, 0.0, 1.0, transparency);
        case 1:
            return Vec4(x2, x2, 1.0, transparency);
        case 2:
            return Vec4(1.0, 2.0 - x2, 2.0 - x2, transparency);
        }
        return Vec4(1.0, 0.0, 0.0, transparency);
    }

    void update_sphere_euclidean(SurfaceParameters &p, PVertex sphere)
    {
        uint32 sphere_index = index_of(*p.spheres_, sphere);

        const std::vector<SVertex> &cluster = (*p.spheres_cluster_)[sphere_index];

        Vec3 c = (*p.spheres_position_)[sphere_index];
        Scalar r = (*p.spheres_radius_)[sphere_index];

        Eigen::MatrixXd J(cluster.size(), 4);
        J.setZero();
        Eigen::VectorXd b(cluster.size());
        b.setZero();
        uint32 idx = 0;
        Eigen::VectorXd s(4);
        s << c[0], c[1], c[2], r;
        for (uint32 i = 0; i < 10; ++i)
        {
            idx = 0;
            for (SVertex v : cluster)
            {
                uint32 v_index = index_of(*p.surface_, v);
                const Vec3 &pos = (*p.surface_vertex_position_)[v_index];

                Vec3 d = pos - Vec3(s(0), s(1), s(2));
                Scalar l = d.norm();
                if (p.point_cloud_mode_)
                {
                    Scalar a = sqrt((*p.surface_vertex_area_pc_)[v_index]);
                    J.row(idx) = Eigen::Vector4d(-(d[0] / l), -(d[1] / l), -(d[2] / l), -1.0) * a;
                    b(idx) = -(l - s(3)) * a;
                }
                else
                {
                    Scalar a = sqrt((*p.surface_vertex_area_surf_)[v_index]);
                    J.row(idx) = Eigen::Vector4d(-(d[0] / l), -(d[1] / l), -(d[2] / l), -1.0) * a;
                    b(idx) = -(l - s(3)) * a;
                }
                ++idx;
            };

            Eigen::LDLT<Eigen::MatrixXd> solver(J.transpose() * J);
            Eigen::VectorXd delta_s = solver.solve(J.transpose() * b);
            s += delta_s;
            if (delta_s.norm() < 1e-6) // stop early if converged
                break;
        }

        c = s.head<3>();
        r = s[3];

        (*p.spheres_position_)[sphere_index] = c;
        (*p.spheres_radius_)[sphere_index] = r;
    }

    void update_sphere_sqem(SurfaceParameters &p, PVertex sphere)
    {
        uint32 sphere_index = index_of(*p.spheres_, sphere);

        const std::vector<SVertex> &cluster = (*p.spheres_cluster_)[sphere_index];

        Vec3 c = (*p.spheres_position_)[sphere_index];
        Scalar r = (*p.spheres_radius_)[sphere_index];

        Eigen::MatrixXd J(2 * cluster.size(), 4);
        J.setZero();
        Eigen::VectorXd b(2 * cluster.size());
        b.setZero();
        uint32 idx = 0;
        Eigen::VectorXd s(4);
        s << c[0], c[1], c[2], r;
        for (uint32 i = 0; i < 10; ++i)
        {
            idx = 0;
            for (SVertex v : cluster)
            {
                uint32 v_index = index_of(*p.surface_, v);
                const Vec3 &pos = (*p.surface_vertex_position_)[v_index];

                // SQEM energy
                if (p.point_cloud_mode_)
                {
                    Eigen::Vector4d lhs = Eigen::Vector4d::Zero();
                    Scalar rhs = 0.0;
                    const Vec3 &n = (*p.surface_vertex_normal_)[v_index];
                    Vec4 n4 = Vec4(n.x(), n.y(), n.z(), 1.0);
                    Scalar a = sqrt((*p.surface_vertex_area_pc_)[v_index] / (k + 1.0));
                    lhs += -n4 * a;
                    rhs += -1.0 * ((pos - Vec3(s(0), s(1), s(2))).dot(n) - s(3)) * a;
                    for (SVertex vn : (*p.surface_vertex_knn_)[v_index])
                    {
                        uint32 vn_index = index_of(*p.surface_, vn);
                        const Vec3 &pn = (*p.surface_vertex_position_)[vn_index];
                        const Vec3 &nn = (*p.surface_vertex_normal_)[vn_index];
                        Vec4 nn4 = Vec4(nn.x(), nn.y(), nn.z(), 1.0);
                        Scalar an = sqrt((*p.surface_vertex_area_pc_)[vn_index] / (k + 1.0));
                        lhs += -nn4 * an;
                        rhs += -1.0 * ((pn - Vec3(s(0), s(1), s(2))).dot(nn) - s(3)) * an;
                    }
                    J.row(idx) = lhs;
                    b(idx) = rhs;
                }
                else
                {
                    Eigen::Vector4d lhs = Eigen::Vector4d::Zero();
                    Scalar rhs = 0.0;
                    foreach_incident_face(*p.surface_, v, [&](SFace f) -> bool {
                        uint32 f_index = index_of(*p.surface_, f);
                        const Vec3 &n = (*p.surface_face_normal_)[f_index];
                        Vec4 n4 = Vec4(n.x(), n.y(), n.z(), 1.0);
                        Scalar a = sqrt((*p.surface_face_area_)[f_index] / 3.0);
                        lhs += -n4 * a;
                        rhs += -1.0 * ((pos - Vec3(s(0), s(1), s(2))).dot(n) - s(3)) * a;
                        return true;
                    });
                    J.row(idx) = lhs;
                    b(idx) = rhs;
                }
                ++idx;

                // distance energy
                Vec3 d = pos - Vec3(s(0), s(1), s(2));
                Scalar l = d.norm();
                if (p.point_cloud_mode_)
                {
                    Scalar a = sqrt((*p.surface_vertex_area_pc_)[v_index]);
                    J.row(idx) =
                        Eigen::Vector4d(-(d[0] / l), -(d[1] / l), -(d[2] / l), -1.0) * a * p.sqem_update_lambda_;
                    b(idx) = -(l - s(3)) * a * p.sqem_update_lambda_; // scale the row by the update lambda
                }
                else
                {
                    Scalar a = sqrt((*p.surface_vertex_area_surf_)[v_index]);
                    J.row(idx) =
                        Eigen::Vector4d(-(d[0] / l), -(d[1] / l), -(d[2] / l), -1.0) * a * p.sqem_update_lambda_;
                    b(idx) = -(l - s(3)) * a * p.sqem_update_lambda_; // scale the row by the update lambda
                }
                ++idx;
            };

            Eigen::LDLT<Eigen::MatrixXd> solver(J.transpose() * J);
            Eigen::VectorXd delta_s = solver.solve(J.transpose() * b);
            s += delta_s;
            if (delta_s.norm() < 1e-6) // stop early if converged
                break;
        }

        c = s.head<3>();
        r = s[3];

        (*p.spheres_position_)[sphere_index] = c;
        (*p.spheres_radius_)[sphere_index] = r;
    }

    void correct_sphere(SurfaceParameters &p, PVertex v)
    {
        uint32 v_index = index_of(*p.spheres_, v);

        Vec3 &c = (*p.spheres_position_)[v_index];
        Scalar &r = (*p.spheres_radius_)[v_index];

        Vec3 closest_point_position;
        Vec3 closest_point_dir;

        if (p.point_cloud_mode_)
        {
            std::pair<uint32, Scalar> k_res;
            p.surface_kdt_->find_nn(c, &k_res);
            SVertex closest_vertex = p.surface_kdt_vertices_[k_res.first];
            closest_point_position = p.surface_kdt_->vertex(k_res.first);
            closest_point_dir = (closest_point_position - c).normalized();

            const Vec3 &closest_vertex_normal = value<Vec3>(*p.surface_, p.surface_vertex_normal_, closest_vertex);
            // TODO: exterior detection is not reliable
            if (closest_point_dir.dot(closest_vertex_normal) <= 0.0)
                closest_point_dir = -closest_point_dir;
        }
        else
        {
            std::pair<uint32, Vec3> bvh_res;
            p.surface_bvh_->closest_point(c, &bvh_res);
            closest_point_position = bvh_res.second;
            closest_point_dir = (closest_point_position - c).normalized();

            const Vec3 &closest_face_normal =
                value<Vec3>(*p.surface_, p.surface_face_normal_, p.surface_bvh_faces_[bvh_res.first]);
            // TODO: exterior detection is not reliable
            if (closest_point_dir.dot(closest_face_normal) <= 0.0)
                closest_point_dir = -closest_point_dir;
        }

        auto [center, radius, secondary] = geometry::shrinking_ball_center(
            *p.surface_, closest_point_position, closest_point_dir, p.surface_vertex_position_.get(), p.surface_bvh_,
            p.surface_bvh_faces_, p.surface_kdt_, p.surface_kdt_vertices_, p.point_cloud_mode_, 0.25f);

        c = center;
        r = radius;
    }

    void update_spheres(SurfaceParameters &p)
    {
        compute_clusters(p);
        parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
            if (p.use_sqem_term_)
                update_sphere_sqem(p, v);
            else
                update_sphere_euclidean(p, v);
            if (p.sphere_correction_ && p.sphere_correction_mode_ == CORRECT_ALWAYS)
                correct_sphere(p, v);
            value<bool>(*p.spheres_, p.spheres_do_not_split_, v) = false;
            return true;
        });

        compute_spheres_error(p); // compute spheres error
        // std::cout << p.total_error_not_normalized_ << std::endl;

        if (p.auto_split_ &&
            (p.total_error_diff_ < 1e-5 || p.iteration_count_ % 10 == 0)) // wait for convergence or max 10 iterations
        {
            switch (p.auto_split_mode_)
            {
            case ERROR_THRESHOLD: {
                if (p.max_error_ > p.auto_split_error_threshold_)
                {
                    compute_skeleton(p, true); // only neighbor clusters computation

                    if (p.sphere_correction_ && p.sphere_correction_mode_ == CORRECT_ON_SPLIT)
                    {
                        parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
                            correct_sphere(p, v);
                            return true;
                        });
                    }

                    std::vector<PVertex> sorted_spheres;
                    sorted_spheres.reserve(p.nb_spheres_);
                    foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
                        sorted_spheres.push_back(v);
                        return true;
                    });
                    std::sort(sorted_spheres.begin(), sorted_spheres.end(), [&](PVertex a, PVertex b) {
                        return value<Scalar>(*p.spheres_, p.spheres_error_, a) >
                               value<Scalar>(*p.spheres_, p.spheres_error_, b);
                    });
                    // do not split more than 20% of the spheres at once (and 10 at most)
                    uint32 to_split_max = std::min(uint32(std::ceil(p.nb_spheres_ * 0.2)), 10u);
                    for (PVertex sphere : sorted_spheres)
                    {
                        uint32 s_index = index_of(*p.spheres_, sphere);
                        Scalar error = (*p.spheres_error_)[s_index];
                        if (error < p.auto_split_error_threshold_)
                            break;
                        if (to_split_max > 0 && !(*p.spheres_do_not_split_)[s_index]) // do not split neighbor spheres
                        {
                            for (PVertex neighbor : (*p.spheres_neighbor_clusters_)[s_index])
                                value<bool>(*p.spheres_, p.spheres_do_not_split_, neighbor) = true;
                            split_sphere(p, sphere);
                            --to_split_max;
                        }
                    }
                }
            }
            break;
            case MAX_NB_SPHERES: {
                if (p.nb_spheres_ < p.auto_split_max_nb_spheres_)
                {
                    compute_skeleton(p, true); // only neighbor clusters computation

                    if (p.sphere_correction_ && p.sphere_correction_mode_ == CORRECT_ON_SPLIT)
                    {
                        parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
                            correct_sphere(p, v);
                            return true;
                        });
                    }

                    std::vector<PVertex> sorted_spheres;
                    sorted_spheres.reserve(p.nb_spheres_);
                    foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
                        sorted_spheres.push_back(v);
                        return true;
                    });
                    std::sort(sorted_spheres.begin(), sorted_spheres.end(), [&](PVertex a, PVertex b) {
                        return value<Scalar>(*p.spheres_, p.spheres_error_, a) >
                               value<Scalar>(*p.spheres_, p.spheres_error_, b);
                    });
                    // do not split more than 20% of the spheres at once (and 10 at most)
                    uint32 to_split_max = std::min(uint32(std::ceil(p.nb_spheres_ * 0.2)), 10u);
                    for (PVertex sphere : sorted_spheres)
                    {
                        uint32 s_index = index_of(*p.spheres_, sphere);
                        if (p.auto_split_max_nb_spheres_ - p.nb_spheres_ <= 0)
                            break;
                        if (to_split_max > 0 && !(*p.spheres_do_not_split_)[s_index]) // do not split neighbor spheres
                        {
                            for (PVertex neighbor : (*p.spheres_neighbor_clusters_)[s_index])
                                value<bool>(*p.spheres_, p.spheres_do_not_split_, neighbor) = true;
                            split_sphere(p, sphere);
                            --to_split_max;
                        }
                    }
                }
            }
            break;
            }
        }

        if (!p.running_)
            update_render_data(p);
    }

    void split_sphere(SurfaceParameters &p, PVertex v)
    {
        uint32 v_index = index_of(*p.spheres_, v);

        // find the vertex of the cluster with maximal error w.r.t. the sphere

        SVertex max_error_vertex;
        Scalar max_error_vertex_error = 0.0;
        const std::vector<SVertex> &cluster = (*p.spheres_cluster_)[v_index];
        for (SVertex sv : cluster)
        {
            Scalar error = value<Scalar>(*p.surface_, p.surface_vertex_error_, sv);
            if (error > max_error_vertex_error)
            {
                max_error_vertex_error = error;
                max_error_vertex = sv;
            }
        }
        uint32 max_error_vertex_index = index_of(*p.surface_, max_error_vertex);

        // insert a new sphere for this vertex

        PVertex new_sphere = add_vertex(*p.spheres_);
        p.nb_spheres_++;
        uint32 new_sphere_index = index_of(*p.spheres_, new_sphere);
        (*p.spheres_position_)[new_sphere_index] = (*p.medial_axis_position_)[max_error_vertex_index];
        (*p.spheres_radius_)[new_sphere_index] = (*p.medial_axis_radius_)[max_error_vertex_index];

        // sample a random bright pastel color for the new sphere
        (*p.spheres_cluster_color_)[new_sphere_index] = Vec3(
            0.5 + 0.5 * (rand() % 256) / 256.0, 0.5 + 0.5 * (rand() % 256) / 256.0, 0.5 + 0.5 * (rand() % 256) / 256.0);
    }

    void remove_sphere(SurfaceParameters &p, PVertex v)
    {
        const std::vector<SVertex> &cluster = value<std::vector<SVertex>>(*p.spheres_, p.spheres_cluster_, v);
        for (SVertex sv : cluster)
            value<PVertex>(*p.surface_, p.surface_vertex_sphere_, sv) = PVertex();
        remove_vertex(*p.spheres_, v);
        p.nb_spheres_--;
    }

    struct edge_hash
    {
        std::size_t operator()(const std::pair<uint32, uint32> &edge) const
        {
            return std::hash<uint32>()(edge.first) + std::hash<uint32>()(edge.second);
        }
    };

    struct edge_equal
    {
        bool operator()(const std::pair<uint32, uint32> &edge1, const std::pair<uint32, uint32> &edge2) const
        {
            return ((edge1.first == edge2.first && edge1.second == edge2.second) ||
                    (edge1.first == edge2.second && edge1.second == edge2.first));
        }
    };

    void compute_skeleton(SurfaceParameters &p, bool compute_neighbor_clusters_only = false)
    {
        // clean neighbor clusters sets
        parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
            uint32 v_index = index_of(*p.spheres_, v);
            (*p.spheres_neighbor_clusters_)[v_index].clear();
            return true;
        });

        // compute neighbor clusters
        if (p.point_cloud_mode_)
        {
            foreach_cell(*p.surface_, [&](SVertex v) -> bool {
                uint32 v_index = index_of(*p.surface_, v);
                PVertex v_sphere = (*p.surface_vertex_sphere_)[v_index];
                for (SVertex w : (*p.surface_vertex_knn_)[v_index])
                {
                    PVertex w_sphere = value<PVertex>(*p.surface_, p.surface_vertex_sphere_, w);
                    if (v_sphere.is_valid() && w_sphere.is_valid() && v_sphere != w_sphere)
                    {
                        value<std::set<PVertex>>(*p.spheres_, p.spheres_neighbor_clusters_, v_sphere).insert(w_sphere);
                        value<std::set<PVertex>>(*p.spheres_, p.spheres_neighbor_clusters_, w_sphere).insert(v_sphere);
                    }
                }
                return true;
            });
        }
        else
        {
            foreach_cell(*p.surface_, [&](SEdge e) -> bool {
                std::vector<SVertex> vertices = incident_vertices(*p.surface_, e);
                PVertex v1_sphere = value<PVertex>(*p.surface_, p.surface_vertex_sphere_, vertices[0]);
                PVertex v2_sphere = value<PVertex>(*p.surface_, p.surface_vertex_sphere_, vertices[1]);
                if (v1_sphere.is_valid() && v2_sphere.is_valid() && v1_sphere != v2_sphere)
                {
                    value<std::set<PVertex>>(*p.spheres_, p.spheres_neighbor_clusters_, v1_sphere).insert(v2_sphere);
                    value<std::set<PVertex>>(*p.spheres_, p.spheres_neighbor_clusters_, v2_sphere).insert(v1_sphere);
                }
                return true;
            });
        }

        if (compute_neighbor_clusters_only)
            return;

        clear(*p.skeleton_);

        auto spheres_skeleton_vertex_map =
            add_attribute<NMVertex, PVertex>(*p.spheres_, "__spheres_skeleton_vertex_map");

        foreach_cell(*p.spheres_, [&](PVertex pv) -> bool {
            uint32 pv_index = index_of(*p.spheres_, pv);
            NMVertex nmv = add_vertex(*p.skeleton_);
            value<Vec3>(*p.skeleton_, p.skeleton_position_, nmv) = (*p.spheres_position_)[pv_index];
            (*spheres_skeleton_vertex_map)[pv_index] = nmv;
            return true;
        });

        std::unordered_map<std::pair<uint32, uint32>, NMEdge, edge_hash, edge_equal> edge_indices;

        foreach_cell(*p.spheres_, [&](PVertex pv) -> bool {
            uint32 pv_index = index_of(*p.spheres_, pv);
            NMVertex nmv1 = (*spheres_skeleton_vertex_map)[pv_index];
            const std::set<PVertex> &neighbors = (*p.spheres_neighbor_clusters_)[pv_index];
            for (PVertex neighbor : neighbors)
            {
                NMVertex nmv2 = value<NMVertex>(*p.spheres_, spheres_skeleton_vertex_map, neighbor);
                std::vector<NMVertex> av = adjacent_vertices_through_edge(*p.skeleton_, nmv1);
                if (std::find(av.begin(), av.end(), nmv2) == av.end())
                {
                    NMEdge e = add_edge(*p.skeleton_, nmv1, nmv2);
                    edge_indices[{index_of(*p.skeleton_, nmv1), index_of(*p.skeleton_, nmv2)}] = e;
                }
            }
            return true;
        });

        foreach_cell(*p.spheres_, [&](PVertex pv) -> bool {
            NMVertex nmv1 = value<NMVertex>(*p.spheres_, spheres_skeleton_vertex_map, pv);
            const std::set<PVertex> &n_pv = value<std::set<PVertex>>(*p.spheres_, p.spheres_neighbor_clusters_, pv);
            for (const PVertex &ne1 : n_pv)
            {
                NMVertex nmv2 = value<NMVertex>(*p.spheres_, spheres_skeleton_vertex_map, ne1);
                const std::set<PVertex> &ne_ne1 =
                    value<std::set<PVertex>>(*p.spheres_, p.spheres_neighbor_clusters_, ne1);
                for (const PVertex &ne2 : ne_ne1)
                {
                    if (n_pv.find(ne2) != n_pv.end())
                    {
                        NMVertex nmv3 = value<NMVertex>(*p.spheres_, spheres_skeleton_vertex_map, ne2);
                        if (index_of(*p.skeleton_, nmv1) < index_of(*p.skeleton_, nmv2) &&
                            index_of(*p.skeleton_, nmv2) < index_of(*p.skeleton_, nmv3))
                        {
                            std::vector<NMEdge> edges;
                            edges.reserve(3);
                            edges.push_back(edge_indices[{index_of(*p.skeleton_, nmv1), index_of(*p.skeleton_, nmv2)}]);
                            edges.push_back(edge_indices[{index_of(*p.skeleton_, nmv2), index_of(*p.skeleton_, nmv3)}]);
                            edges.push_back(edge_indices[{index_of(*p.skeleton_, nmv1), index_of(*p.skeleton_, nmv3)}]);
                            add_face(*p.skeleton_, edges);
                        }
                    }
                }
            }
            return true;
        });

        remove_attribute<PVertex>(*p.spheres_, spheres_skeleton_vertex_map);
    }

  protected:
    void init() override
    {
        surface_provider_ = static_cast<ui::MeshProvider<SURFACE> *>(
            app_.module("MeshProvider (" + std::string{mesh_traits<SURFACE>::name} + ")"));

        points_provider_ = static_cast<ui::MeshProvider<POINTS> *>(
            app_.module("MeshProvider (" + std::string{mesh_traits<POINTS>::name} + ")"));

        non_manifold_provider_ = static_cast<ui::MeshProvider<NONMANIFOLD> *>(
            app_.module("MeshProvider (" + std::string{mesh_traits<NONMANIFOLD>::name} + ")"));

        timer_connection_ = boost::synapse::connect<App::timer_tick>(&app_, [this]() {
            SurfaceParameters &p = surface_parameters_[selected_surface_];
            update_render_data(p);
        });
    }

    void update_render_data(SurfaceParameters &p)
    {
        if (p.running_)
        {
            std::lock_guard<std::mutex> lock(p.mutex_);
            points_provider_->emit_connectivity_changed(*p.spheres_);
            points_provider_->emit_attribute_changed(*p.spheres_, p.spheres_position_.get());
            points_provider_->emit_attribute_changed(*p.spheres_, p.spheres_radius_.get());

            update_spheres_color(p);
            points_provider_->emit_attribute_changed(*p.spheres_, p.spheres_color_.get());

            parallel_foreach_cell(*p.surface_, [&](SVertex v) -> bool {
                uint32 v_index = index_of(*p.surface_, v);
                PVertex sphere = (*p.surface_vertex_sphere_)[v_index];
                if (sphere.is_valid())
                    (*p.surface_vertex_color_)[v_index] = value<Vec3>(*p.spheres_, p.spheres_cluster_color_, sphere);
                return true;
            });
            surface_provider_->emit_attribute_changed(*p.surface_, p.surface_vertex_color_.get());

            compute_skeleton(p);
        }
        else
        {
            points_provider_->emit_connectivity_changed(*p.spheres_);
            points_provider_->emit_attribute_changed(*p.spheres_, p.spheres_position_.get());
            points_provider_->emit_attribute_changed(*p.spheres_, p.spheres_radius_.get());

            update_spheres_color(p);
            points_provider_->emit_attribute_changed(*p.spheres_, p.spheres_color_.get());

            parallel_foreach_cell(*p.surface_, [&](SVertex v) -> bool {
                uint32 v_index = index_of(*p.surface_, v);
                PVertex sphere = (*p.surface_vertex_sphere_)[v_index];
                if (sphere.is_valid())
                    (*p.surface_vertex_color_)[v_index] = value<Vec3>(*p.spheres_, p.spheres_cluster_color_, sphere);
                return true;
            });
            surface_provider_->emit_attribute_changed(*p.surface_, p.surface_vertex_color_.get());

            compute_skeleton(p);
        }

        non_manifold_provider_->emit_connectivity_changed(*p.skeleton_);
        non_manifold_provider_->emit_attribute_changed(*p.skeleton_, p.skeleton_position_.get());
    }

    void start_spheres_update(SurfaceParameters &p)
    {
        p.running_ = true;
        p.iteration_count_ = 0;
        p.total_error_diff_ = 0.0;
        p.last_total_error_ = std::numeric_limits<Scalar>::max();

        launch_thread([&]() {
            auto start = std::chrono::high_resolution_clock::now();
            while (true)
            {
                {
                    std::lock_guard<std::mutex> lock(p.mutex_);
                    update_spheres(p);
                    p.iteration_count_++;
                }
                if (p.slow_down_)
                    std::this_thread::sleep_for(std::chrono::microseconds(1000000 / p.update_rate_));
                else
                    std::this_thread::yield();

                if (p.auto_stop_)
                {
                    if (p.total_error_diff_ < 1e-5)
                    {
                        switch (p.auto_split_mode_)
                        {
                        case ERROR_THRESHOLD:
                            if (p.max_error_ < p.auto_split_error_threshold_)
                                p.stopping_ = true;
                            break;
                        case MAX_NB_SPHERES:
                            if (p.nb_spheres_ >= p.auto_split_max_nb_spheres_)
                                p.stopping_ = true;
                            break;
                        }
                    }
                }

                if (p.stopping_)
                {
                    p.stopping_ = false;
                    p.running_ = false;
                    break;
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Sphere optimizations time: " << std::chrono::duration<Scalar>(end - start).count() << "s"
                      << std::endl;
            std::cout << "Nb iterations: " << p.iteration_count_ << std::endl;
        });

        app_.start_timer(100, [&]() -> bool { return !p.running_; });
    }

    void stop_spheres_update(SurfaceParameters &p)
    {
        p.stopping_ = true;
    }

    void key_press_event(View *view, int32 key_code) override
    {
       
        if (ImGui::GetIO().WantCaptureKeyboard || ImGui::IsAnyItemActive())
            return;
            
        SurfaceParameters &p = surface_parameters_[selected_surface_];

        if (key_code == GLFW_KEY_U)
        {
            if (!p.running_)
            {
                std::lock_guard<std::mutex> lock(p.mutex_);
                update_render_data(p);
            }
        }
        else if (key_code == GLFW_KEY_I || key_code == GLFW_KEY_S || key_code == GLFW_KEY_D)
        {
            int32 x = view->mouse_x();
            int32 y = view->mouse_y();

            // minwindef.h (included by MSVC) defines near and far macros, which is why the underscore is needed
            rendering::GLVec3d near_ = view->unproject(x, y, 0.0);
            rendering::GLVec3d far_d = view->unproject(x, y, 1.0);
            Vec3 A{near_.x(), near_.y(), near_.z()};
            Vec3 B{far_d.x(), far_d.y(), far_d.z()};

            Vec3 picked_sphere_center;
            foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
                if (!picked_sphere_.is_valid())
                {
                    picked_sphere_ = v;
                    picked_sphere_center = value<Vec3>(*p.spheres_, p.spheres_position_, picked_sphere_);
                    return true;
                }
                const Vec3 &sp = value<Vec3>(*p.spheres_, p.spheres_position_, v);
                if (geometry::squared_distance_line_point(A, B, sp) <
                    geometry::squared_distance_line_point(A, B, picked_sphere_center))
                {
                    picked_sphere_ = v;
                    picked_sphere_center = sp;
                }
                return true;
            });

            if (key_code == GLFW_KEY_S && picked_sphere_.is_valid())
            {
                std::lock_guard<std::mutex> lock(p.mutex_);
                split_sphere(p, picked_sphere_);
                compute_clusters(p);
                compute_spheres_error(p);
                if (!p.running_)
                    update_render_data(p);
            }
            else if (key_code == GLFW_KEY_D && picked_sphere_.is_valid())
            {
                std::lock_guard<std::mutex> lock(p.mutex_);
                remove_sphere(p, picked_sphere_);
                compute_clusters(p);
                compute_spheres_error(p);
                if (!p.running_)
                    update_render_data(p);
            }
        }
    }

    void left_panel() override
    {
        imgui_mesh_selector(surface_provider_, selected_surface_, "Surface",
                            [&](SURFACE &s) { set_selected_surface(s); });

        if (selected_surface_)
        {
            SurfaceParameters &p = surface_parameters_[selected_surface_];

            imgui_combo_attribute<SVertex, Vec3>(
                *selected_surface_, p.surface_vertex_position_, "Position",
                [&](const std::shared_ptr<SAttribute<Vec3>> &attribute) { p.surface_vertex_position_ = attribute; });

            if (p.surface_vertex_position_ && !p.initialized_)
            {
                if (ImGui::Button("Init surface data"))
                    init_surface_data(*selected_surface_);
            }

            if (p.initialized_)
            {
				if (ImGui::Button("Init spheres"))
				{
					std::lock_guard<std::mutex> lock(p.mutex_);
					init_spheres(*selected_surface_, p.init_max_nb_spheres_);
				}
				ImGui::Checkbox("Auto split", &p.auto_split_);
				if (p.auto_split_)
				{
					ImGui::RadioButton("Error threshold", (int*)&p.auto_split_mode_, ERROR_THRESHOLD);
					ImGui::SameLine();
					ImGui::RadioButton("Max nb sphere", (int*)&p.auto_split_mode_, MAX_NB_SPHERES);
					if (p.auto_split_mode_ == ERROR_THRESHOLD)
						ImGui::SliderFloat("Threshold", &p.auto_split_error_threshold_, 0.0f, 1.0f, "%.6f",
							ImGuiSliderFlags_Logarithmic);
					else
						ImGui::InputScalar("Nb spheres", ImGuiDataType_U32, &p.auto_split_max_nb_spheres_);
				}

				
				if (!p.running_)
				{
					if (ImGui::Button("Start spheres update"))
						start_spheres_update(p);
				}
				else
				{
					if (ImGui::Button("Stop spheres update"))
						stop_spheres_update(p);
				}
				
                if (ImGui::CollapsingHeader("Experiments")) {
					ImGui::Text("Init strategy: Coverage Axis like init strategy.");
                    ImGui::Text("After changed the value, click \"Init spheres\"button");
					ImGui::SliderFloat("Init dilation factor", &p.init_dilation_factor_, 1.0, 4.0);
					
					ImGui::InputScalar("Init nb spheres", ImGuiDataType_U32, &p.init_max_nb_spheres_);
					
                    ImGui::Separator();
                    ImGui::Text("Generate skeleton with only vertex information");
					if (ImGui::Checkbox("Point cloud mode", &p.point_cloud_mode_))
					{
						std::lock_guard<std::mutex> lock(p.mutex_);
						compute_quadrics(p);
					}
                    ImGui::Separator();
					ImGui::Text("Do we slow down the process?");
					ImGui::Checkbox("Slow down", &p.slow_down_);
					if (p.slow_down_)
						ImGui::SliderInt("Update rate", (int*)&p.update_rate_, 1, 1000);

					ImGui::Checkbox("Auto stop", &p.auto_stop_);
                    ImGui::Separator();
					static bool sync_lambda = true;
					ImGui::Checkbox("Sync lambda", &sync_lambda);
					if (ImGui::SliderFloat("update lambda", &p.sqem_update_lambda_, 0.0f, 2.0f, "%.6f"))
					{
						if (sync_lambda)
							p.sqem_clustering_lambda_ = p.sqem_update_lambda_;
					}
					if (ImGui::SliderFloat("clustering lambda", &p.sqem_clustering_lambda_, 0.0f, 2.0f, "%.6f"))
					{
						if (sync_lambda)
							p.sqem_update_lambda_ = p.sqem_clustering_lambda_;
					}
					ImGui::Separator();

                    //ImGui::Checkbox("Use SQEM term", &p.use_sqem_term_);
                    ImGui::Text("Mannual operation..");
                    if (ImGui::Button("Update spheres"))
                    {
                        if (!p.running_)
                        {
                            std::lock_guard<std::mutex> lock(p.mutex_);
                            update_spheres(p);
                            compute_spheres_error(p);
                            update_render_data(p);
                        }
                    }

                    if (ImGui::Button("Compute clusters"))
                    {
                        if (!p.running_)
                        {
                            std::lock_guard<std::mutex> lock(p.mutex_);
                            compute_clusters(p);
                            compute_spheres_error(p);
                            update_render_data(p);
                        }
                    }
                    ImGui::Separator();
					ImGui::Text("Correct or not? This is a question..");
                    ImGui::Checkbox("Sphere correction step", &p.sphere_correction_);
                    if (p.sphere_correction_)
                    {
                        ImGui::RadioButton("Always", (int*)&p.sphere_correction_mode_, CORRECT_ALWAYS);
                        ImGui::SameLine();
                        ImGui::RadioButton("On split", (int*)&p.sphere_correction_mode_, CORRECT_ON_SPLIT);
                    }

                    if (ImGui::Button("Correct spheres"))
                    {
                        if (!p.running_)
                        {
                            std::lock_guard<std::mutex> lock(p.mutex_);
                            parallel_foreach_cell(*p.spheres_, [&](PVertex v) -> bool {
                                correct_sphere(p, v);
                                return true;
                                });
                            compute_spheres_error(p);
                            update_render_data(p);
                        }
                    }
					if (ImGui::Checkbox("Error as color", &p.error_as_spheres_color_))
					{
						std::lock_guard<std::mutex> lock(p.mutex_);
						if (!p.running_)
							update_render_data(p);
					}
					if (ImGui::Button("Split max error sphere"))
					{
						std::lock_guard<std::mutex> lock(p.mutex_);
						split_sphere(p, p.max_error_sphere_);
						compute_clusters(p);
						compute_spheres_error(p);
						if (!p.running_)
							update_render_data(p);
					}
                    ImGui::Separator();

                    
                    if (ImGui::SliderFloat("Transparency", &p.spheres_transparency_, 0.0f, 1.0f))
                    {
                        std::lock_guard<std::mutex> lock(p.mutex_);
                        if (!p.running_)
                            update_render_data(p);
                    }

					ImGui::Separator();
					ImGui::Text("Add some noise!");

                    ImGui::SliderFloat("Noise factor", &p.noise_factor_, 0.0f, 0.1f, "%.6f");
                    if (ImGui::Button("Add noise"))
                    {
                        std::lock_guard<std::mutex> lock(p.mutex_);
                        add_surface_noise(p);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Restore position"))
                    {
                        std::lock_guard<std::mutex> lock(p.mutex_);
                        restore_surface_position(p);
                    }
                }
                ImGui::Separator();

                ImGui::Text("Total error: %f", p.total_error_);
                ImGui::Text("Min error: %f", p.min_error_);
                ImGui::Text("Max error: %f", p.max_error_);

                ImGui::Separator();

                ImGui::Text("Pick the sphere under the mouse with I to get information");
                ImGui::Text("Interactive editing - S: Split sphere, D: Delete sphere");
                if (picked_sphere_.is_valid())
                {
                    ImGui::Text("Picked sphere:");
                    const Vec3 &sp = value<Vec3>(*p.spheres_, p.spheres_position_, picked_sphere_);
                    ImGui::Text("Center: (%f, %f, %f)", sp[0], sp[1], sp[2]);
                    ImGui::Text("Radius: %f", value<Scalar>(*p.spheres_, p.spheres_radius_, picked_sphere_));
                }
            }

        }

    }

  private:
    MeshProvider<SURFACE> *surface_provider_ = nullptr;
    MeshProvider<POINTS> *points_provider_ = nullptr;
    MeshProvider<NONMANIFOLD> *non_manifold_provider_ = nullptr;

    std::unordered_map<const SURFACE *, SurfaceParameters> surface_parameters_;

    SURFACE *selected_surface_ = nullptr;
    PVertex picked_sphere_;

    std::array<std::mutex, 43> spheres_mutex_;

    std::unordered_map<const SURFACE *, std::vector<std::shared_ptr<boost::synapse::connection>>> surface_connections_;
    std::shared_ptr<boost::synapse::connection> timer_connection_;
};

} // namespace ui

} // namespace cgogn

#endif // CGOGN_MODULE_VMAS_H_
