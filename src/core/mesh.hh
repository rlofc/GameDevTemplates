#ifndef GDT_MESH_HEADER_INCLUDED
#define GDT_MESH_HEADER_INCLUDED

#include <functional>
#include <memory>
#include <vector>

#include "math.hh"

namespace gdt {

struct vertex {
    math::vec3 position;
    math::vec3 normal;
    math::vec3 tangent;
    math::vec3 binormal;
    math::vec4 color = {1.0, 1.0, 1.0, 1.0};
    math::vec2 uvs;
    void to_array(float* p)
    {
        math::vec3_to_array(this->position, &p[0]);
        math::vec3_to_array(this->normal, &p[3]);
        math::vec3_to_array(this->tangent, &p[6]);
        math::vec3_to_array(this->binormal, &p[9]);
        math::vec2_to_array(this->uvs, &p[12]);
        math::vec4_to_array(this->color, &p[14]);
    }
};

static math::vec3 triangle_tangent(vertex vert1, vertex vert2, vertex vert3)
{
    math::vec3 pos1 = vert1.position;
    math::vec3 pos2 = vert2.position;
    math::vec3 pos3 = vert3.position;

    math::vec2 uv1 = vert1.uvs;
    math::vec2 uv2 = vert2.uvs;
    math::vec2 uv3 = vert3.uvs;

    // get component vectors
    float x1 = pos2.x - pos1.x;
    float x2 = pos3.x - pos1.x;
    float y1 = pos2.y - pos1.y;
    float y2 = pos3.y - pos1.y;
    float z1 = pos2.z - pos1.z;
    float z2 = pos3.z - pos1.z;

    // Gen UV space vectors
    float s1 = uv2.x - uv1.x;
    float s2 = uv3.x - uv1.x;
    float t1 = uv2.y - uv1.y;
    float t2 = uv3.y - uv1.y;

    float r = 1.0f / ((s1 * t2) - (s2 * t1));

    math::vec3 tdir =
        math::vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

    return tdir.normalize();
}

static math::vec3 triangle_binormal(vertex vert1, vertex vert2, vertex vert3)
{
    math::vec3 pos1 = vert1.position;
    math::vec3 pos2 = vert2.position;
    math::vec3 pos3 = vert3.position;

    math::vec2 uv1 = vert1.uvs;
    math::vec2 uv2 = vert2.uvs;
    math::vec2 uv3 = vert3.uvs;

    // get component vectors
    float x1 = pos2.x - pos1.x;
    float x2 = pos3.x - pos1.x;
    float y1 = pos2.y - pos1.y;
    float y2 = pos3.y - pos1.y;
    float z1 = pos2.z - pos1.z;
    float z2 = pos3.z - pos1.z;

    // Gen UV space vectors
    float s1 = uv2.x - uv1.x;
    float s2 = uv3.x - uv1.x;
    float t1 = uv2.y - uv1.y;
    float t2 = uv3.y - uv1.y;

    float r = 1.0f / ((s1 * t2) - (s2 * t1));

    math::vec3 sdir =
        math::vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);

    return sdir.normalize();
}

static math::vec3 triangle_normal(vertex v1, vertex v2, vertex v3)
{
    math::vec3 edge1 = (v2.position - v1.position);
    math::vec3 edge2 = (v3.position - v1.position);
    math::vec3 normal = edge1.cross(edge2);
    return normal.normalize();
}

static float triangle_area(vertex v1, vertex v2, vertex v3)
{
    math::vec3 ab = (v1.position - v2.position);
    math::vec3 ac = (v1.position - v3.position);
    float area = 0.5 * ab.cross(ac).length();
    return area;
}

inline bool operator==(const vertex& v1, const vertex& v2)
{
    if (!(v1.position == v2.position)) {
        return false;
    }
    if (!(v1.normal == v2.normal)) {
        return false;
    }
    if (!(v1.uvs == v2.uvs)) {
        return false;
    }
    return true;
}

struct vertex_weights {
    int bone_ids[3] = {0, 0, 0};
    float bone_weights[3] = {0, 0, 0};
};

struct mesh {
    std::vector<vertex> vertices;
    std::vector<uint32_t> triangles;
    bool is_rigged = false;
    std::vector<vertex_weights> weights;

    void generate_tangents()
    {
        // Clear all tangents
        for (auto& v : vertices) {
            v.tangent = math::vec3();
            v.binormal = math::vec3();
        }

        int i = 0;
        while (i < triangles.size()) {
            int t_i1 = this->triangles[i];
            int t_i2 = this->triangles[i + 1];
            int t_i3 = this->triangles[i + 2];

            vertex v1 = this->vertices[t_i1];
            vertex v2 = this->vertices[t_i2];
            vertex v3 = this->vertices[t_i3];

            math::vec3 face_tangent = triangle_tangent(v1, v2, v3);
            math::vec3 face_binormal = triangle_binormal(v1, v2, v3);

            v1.tangent = (face_tangent + v1.tangent);
            v2.tangent = (face_tangent + v2.tangent);
            v3.tangent = (face_tangent + v3.tangent);

            v1.binormal = (face_binormal + v1.binormal);
            v2.binormal = (face_binormal + v2.binormal);
            v3.binormal = (face_binormal + v3.binormal);

            this->vertices[t_i1] = v1;
            this->vertices[t_i2] = v2;
            this->vertices[t_i3] = v3;

            i = i + 3;
        }

        for (auto& v : vertices) {
            v.tangent = v.tangent.normalize();
            v.binormal = v.binormal.normalize();
        }
    }

    void generate_normals()
    {
        for (auto& v : vertices) {
            v.normal = math::vec3();
        }

        int i = 0;
        while (i < triangles.size()) {
            int t_i1 = this->triangles[i];
            int t_i2 = this->triangles[i + 1];
            int t_i3 = this->triangles[i + 2];

            vertex v1 = this->vertices[t_i1];
            vertex v2 = this->vertices[t_i2];
            vertex v3 = this->vertices[t_i3];

            math::vec3 face_normal = triangle_normal(v1, v2, v3);

            v1.normal = (face_normal + v1.normal);
            v2.normal = (face_normal + v2.normal);
            v3.normal = (face_normal + v3.normal);

            this->vertices[t_i1] = v1;
            this->vertices[t_i2] = v2;
            this->vertices[t_i3] = v3;

            i = i + 3;
        }

        for (auto& v : vertices) {
            v.normal = v.normal.normalize();
        }
    }

    void generate_orthagonal_tangents()
    {
        for (auto& v : vertices) {
            v.tangent = math::vec3();
            v.binormal = math::vec3();
        }

        int i = 0;
        while (i < triangles.size()) {
            int t_i1 = this->triangles[i];
            int t_i2 = this->triangles[i + 1];
            int t_i3 = this->triangles[i + 2];

            vertex v1 = this->vertices[t_i1];
            vertex v2 = this->vertices[t_i2];
            vertex v3 = this->vertices[t_i3];

            math::vec3 face_normal = triangle_normal(v1, v2, v3);
            math::vec3 face_binormal_temp = triangle_binormal(v1, v2, v3);

            math::vec3 face_tangent = face_binormal_temp.cross(face_normal).normalize();
            math::vec3 face_binormal = face_tangent.cross(face_normal).normalize();

            v1.tangent = (face_tangent + v1.tangent);
            v2.tangent = (face_tangent + v2.tangent);
            v3.tangent = (face_tangent + v3.tangent);

            v1.binormal = (face_binormal + v1.binormal);
            v2.binormal = (face_binormal + v2.binormal);
            v3.binormal = (face_binormal + v3.binormal);

            this->vertices[t_i1] = v1;
            this->vertices[t_i2] = v2;
            this->vertices[t_i3] = v3;

            i = i + 3;
        }

        for (auto& v : vertices) {
            v.tangent = v.tangent.normalize();
            v.binormal = v.binormal.normalize();
        }
    }

    void generate_texcoords_cylinder()
    {
        math::vec2 unwrap_vector = math::vec2(1, 0);

        float max_height = -99999999;
        float min_height = 99999999;

        for (auto& vtx : vertices) {
            float v = vtx.position.y;
            max_height = math::max(max_height, v);
            min_height = math::min(min_height, v);

            math::vec2 proj_position = math::vec2(vtx.position.x, vtx.position.z);
            math::vec2 from_center = proj_position.normalize();
            float u = (from_center.dot(unwrap_vector) + 1) / 8;

            vtx.uvs = math::vec2(u, v);
        }

        float scale = (max_height - min_height);

        for (auto& vtx : vertices) {
            vtx.uvs = math::vec2(vtx.uvs.x, vtx.uvs.y / scale);
        }
    }
};

struct model {
    std::vector<std::unique_ptr<mesh>> meshes;

    void generate_normals()
    {
        for (auto& m : meshes) m->generate_normals();
    }

    void generate_tangents()
    {
        for (auto& m : meshes) m->generate_tangents();
    }

    void generate_orthagonal_tangents()
    {
        for (auto& m : meshes) m->generate_orthagonal_tangents();
    }

    void generate_texcoords_cylinder()
    {
        for (auto& m : meshes) m->generate_texcoords_cylinder();
    }
};
};

namespace std {
template <>
struct hash<gdt::vertex> {
    std::size_t operator()(const gdt::vertex& v) const
    {
        std::size_t val = abs(v.position.hash() ^ v.normal.hash() ^ v.uvs.hash());
        return val % 4096;
    }
};
};
#endif  // GDT_MESH_HEADER_INCLUDED
