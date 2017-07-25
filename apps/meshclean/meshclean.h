/*
 * Copyright (C) 2015, Simon Fuhrmann
 * TU Darmstadt - Graphics, Capture and Massively Parallel Computing
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD 3-Clause license. See the LICENSE.txt file for details.
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include "util/system.h"
#include "util/timer.h"
#include "util/arguments.h"
#include "mve/mesh.h"
#include "mve/mesh_io.h"
#include "mve/mesh_io_ply.h"
#include "mve/mesh_tools.h"
#include "fssr/mesh_clean.h"

struct AppSettings_mclean
{
    std::string in_mesh;
    std::string out_mesh;
    bool clean_degenerated = true;
    bool delete_scale = false;
    bool delete_conf = false;
    bool delete_colors = false;
    float conf_threshold = 1.0f;
    float conf_percentile = -1.0f;
    int component_size = 1000;
};

template <typename T>
T
percentile (std::vector<T> const& in, float percent)
{
    /* copy the input vector because 'nth_element' will rearrange it */
    std::vector<T> copy = in;
    std::size_t n = static_cast<std::size_t>(percent / 100.0f * in.size());
    std::nth_element(copy.begin(), copy.begin() + n, copy.end());
    return copy[n];
}

void
remove_low_conf_vertices (mve::TriangleMesh::Ptr mesh, float const thres)
{
    mve::TriangleMesh::ConfidenceList const& confs = mesh->get_vertex_confidences();
    std::vector<bool> delete_list(confs.size(), false);
    for (std::size_t i = 0; i < confs.size(); ++i)
    {
        if (confs[i] > thres)
            continue;
        delete_list[i] = true;
    }
    mesh->delete_vertices_fix_faces(delete_list);
}