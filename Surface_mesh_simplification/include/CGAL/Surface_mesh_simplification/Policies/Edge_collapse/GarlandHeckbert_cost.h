// Copyright (c) 2019  GeometryFactory (France). All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s)     : Baskin Burak Senbaslar
//

#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_GARLANDHECKBERT_COST_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_GARLANDHECKBERT_COST_H

#include <CGAL/license/Surface_mesh_simplification.h>

#include <CGAL/Surface_mesh_simplification/internal/Common.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/internal/GarlandHeckbert_core.h>

#include <CGAL/tags.h>

#include <boost/optional/optional.hpp>

#include <utility>

namespace CGAL {
namespace Surface_mesh_simplification {

template <typename TM_>
struct GarlandHeckbert_cost_matrix
{
  typedef internal::GarlandHeckbert_core<TM_>                    GH_Core;
  typedef typename GH_Core::Matrix4x4                            type;
};

template <typename TM_, typename VCM_>
class GarlandHeckbert_cost
{
  typedef TM_                                                    TM;
  typedef VCM_                                                   Vertex_cost_map;

public:
  typedef typename boost::graph_traits<TM>::vertex_descriptor    vertex_descriptor;
  typedef typename boost::graph_traits<TM>::halfedge_descriptor  halfedge_descriptor;

  typedef internal::GarlandHeckbert_core<TM>                     GH_Core;

  typedef typename GH_Core::Matrix4x4                            Matrix4x4;
  typedef typename GH_Core::Col4                                 Col4;

  typedef typename GH_Core::FT                                   FT;
  typedef typename boost::optional<FT>                           Optional_FT;

  // Tells the edge collapse main function that we need to call "initialize"
  // and "update" functions. A bit awkward, but still better than abusing visitors.
  typedef CGAL::Tag_true                                         Update_tag;

  GarlandHeckbert_cost(Vertex_cost_map& vcm,
                       const FT discontinuity_multiplier = 100)
    :
      m_cost_matrices(vcm),
      m_discontinuity_multiplier(discontinuity_multiplier)
  { }

  void initialize(const TM& tmesh) const
  {
    for(const halfedge_descriptor h : halfedges(tmesh))
    {
      const vertex_descriptor v = target(h, tmesh);
      put(m_cost_matrices, v, GH_Core::fundamental_error_quadric(h, tmesh, m_discontinuity_multiplier));
    }
  }

  template <typename Profile>
  boost::optional<typename Profile::FT>
  operator()(const Profile& aProfile,
             const boost::optional<typename Profile::Point>& aPlacement) const
  {
    if(!aPlacement)
      return boost::optional<typename Profile::FT>();

    CGAL_precondition(get(m_cost_matrices, aProfile.v0()) != Matrix4x4());
    CGAL_precondition(get(m_cost_matrices, aProfile.v1()) != Matrix4x4());

    const Matrix4x4 combined_matrix = GH_Core::combine_matrices(get(m_cost_matrices, aProfile.v0()),
                                                                get(m_cost_matrices, aProfile.v1()));
    const Col4 pt = GH_Core::point_to_homogenous_column(*aPlacement);
    const Optional_FT cost = (pt.transpose() * combined_matrix * pt)(0, 0);

    return cost;
  }

  template <typename Profile>
  void update_after_collapse(const Profile& aProfile,
                             const vertex_descriptor new_v) const
  {
    put(m_cost_matrices, new_v,
        GH_Core::combine_matrices(get(m_cost_matrices, aProfile.v0()),
                                  get(m_cost_matrices, aProfile.v1())));
  }

private:
  Vertex_cost_map m_cost_matrices;
  const FT m_discontinuity_multiplier;
};

} // namespace Surface_mesh_simplification
} // namespace CGAL

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_POLICIES_EDGE_COLLAPSE_GARLANDHECKBERT_COST_H
