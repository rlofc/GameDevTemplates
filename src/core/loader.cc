#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>

#include "graphics.hh"
#include "animation.hh"
#include "mesh.hh"
#include "lodepng.hh"

namespace gdt {

class token_reader {
  public:
    virtual void read(std::ifstream& f) = 0;
};

class version_reader : public token_reader {
  public:
    void read(std::ifstream& f) override
    {
        std::uint32_t version;
        f >> version;
    }
};

class nodes_reader : public token_reader {
  public:
    std::vector<int> parents;
    skeleton _skeleton;
    void read(std::ifstream& f) override
    {
        std::string ids, name;
        int id, id2;
        f >> ids;
        while (ids != "end") {
            id = std::stoi(ids);
            f >> name >> id2;

            parents.push_back(id2);
            _skeleton.bones.push_back(bone{name, id2});
            f >> ids;
        }
    }
};

class skeleton_reader : public token_reader {
  public:
    std::vector<frame> _frames;
    nodes_reader* _nr;
    skeleton_reader(nodes_reader* nr) : _nr(nr)
    {
    }

    void read(std::ifstream& f) override
    {
        std::string time_token, ids;
        int time;
        int id;
        float x, y, z, xx, yy, zz;
        f >> ids;
        frame fr;
        while (ids != "end") {
            if (ids == "time") {
                f >> time;
                if (fr.bone_positions.size() > 0) {
                    fr.bake_transforms();
                    _frames.push_back(fr);
                }
                fr.bone_parents.clear();
                fr.bone_positions.clear();
                fr.bone_rotations.clear();
                fr.bone_transforms.clear();
                fr.bone_inv_transforms.clear();
            }
            else {
                id = std::stoi(ids);
                fr.bone_parents.push_back(_nr->parents[id]);
                f >> x >> y >> z >> xx >> yy >> zz;
                fr.bone_positions.push_back(gdt::math::vec3(x, y, z));
                gdt::math::mat4 rm =
                    gdt::math::mat4::rotation_eular(gdt::math::vec3(xx, yy, zz));
                // TODO: consider rotating Y:Z?
                rm = rm.transpose();
                fr.bone_rotations.push_back(rm.as_quat());
            }
            f >> ids;
            if (ids == "end") {
                fr.bake_transforms();
                _frames.push_back(fr);
            }
        }
    }
};

class triangles_reader : public token_reader {
  public:
    std::unordered_map<gdt::vertex, int> vs;
    std::vector<uint32_t> ts;
    std::vector<gdt::vertex> vsl;
    std::vector<gdt::vertex_weights> vwl;
    int vi = 0;

    void read(std::ifstream& f) override
    {
        std::string material;
        int root, nlinks;
        float x, y, z, nx, ny, nz, u, v;
        f >> material;
        while (material != "end") {
            for (int j = 0; j < 3; j++) {
                gdt::vertex vx;
                gdt::vertex_weights vw;
                f >> root >> x >> y >> z >> nx >> ny >> nz >> u >> v >> nlinks;
                if (nlinks == 0) {
                    LOG_WARNING << "Mesh has vertex with no bones, material is: " << material;
                }
                vx.position = {x, y, z};
                vx.normal = {nx, ny, nz};
                vx.uvs = {u, v};
                float total = 0;
                float excess = 0.0;
                for (int wi = 0; wi < nlinks; wi++) {
                    int id;
                    float weight;
                    f >> id >> weight;
                    if (wi <= 2) {
                        vw.bone_ids[wi] = id;
                        vw.bone_weights[wi] = weight;
                        total += weight;
                    } else {
                        excess += weight;
                    }
                }
                if (excess > 0.0) {
                    vw.bone_weights[0] += excess * (vw.bone_weights[0] / total);
                    vw.bone_weights[1] += excess * (vw.bone_weights[1] / total);
                    vw.bone_weights[2] += excess * (vw.bone_weights[2] / total);
                    total = vw.bone_weights[0] + vw.bone_weights[1] + vw.bone_weights[2];
                }
                if (total > 1.00001 || total < 0.99999) {
                    LOG_ERROR << "Total : " << total;
                }
                if (vs.find(vx) == vs.end()) {
                    vsl.push_back(vx);
                    vwl.push_back(vw);
                    ts.push_back(vi);
                    vs[vx] = vi;
                    vi++;
                }
                else {
                    ts.push_back(vs[vx]);
                }
            }
            f >> material;
        }
    }
};

std::unique_ptr<gdt::model> read_smd(const char* filename)
{
    LOG_DEBUG << "Reading model from: " << filename;
    std::map<std::string, token_reader*> readers;

    std::unique_ptr<version_reader> vr = std::make_unique<version_reader>();
    std::unique_ptr<nodes_reader> nr = std::make_unique<nodes_reader>();
    std::unique_ptr<skeleton_reader> sr = std::make_unique<skeleton_reader>(nr.get());
    std::unique_ptr<triangles_reader> tr = std::make_unique<triangles_reader>();

    readers["version"] = vr.get();
    readers["nodes"] = nr.get();
    readers["skeleton"] = sr.get();
    readers["triangles"] = tr.get();

    std::ifstream f(filename, std::ios::in);
    if (!f) throw std::runtime_error("File not found");

    std::string token;

    f >> token;
    while (f.eof() == false) {
        if (readers.find(token) == readers.end()) throw std::runtime_error("Error reading SMD");
        readers[token]->read(f);
        f >> token;
    }
    std::unique_ptr<gdt::model> model = std::make_unique<gdt::model>();
    std::unique_ptr<gdt::mesh> mesh = std::make_unique<gdt::mesh>();

    mesh->vertices = tr->vsl;
    mesh->triangles = tr->ts;
    if (nr->_skeleton.bones.size()==1 && nr->_skeleton.bones[0].name=="\"root\"")
        mesh->is_rigged = false;
    else
        mesh->is_rigged = true;
    mesh->weights = tr->vwl;
    model->meshes.push_back(std::move(mesh));
    model->generate_tangents();
    return std::move(model);
}

std::vector<frame> read_animation(const char* filename)
{
    LOG_DEBUG << "Reading animation file: " << filename;
    std::map<std::string, token_reader*> readers;

    std::unique_ptr<version_reader> vr = std::make_unique<version_reader>();
    std::unique_ptr<nodes_reader> nr = std::make_unique<nodes_reader>();
    std::unique_ptr<skeleton_reader> sr = std::make_unique<skeleton_reader>(nr.get());

    readers["version"] = vr.get();
    readers["nodes"] = nr.get();
    readers["skeleton"] = sr.get();

    std::ifstream f(filename, std::ios::in);
    if (!f) throw std::runtime_error("File not found");

    std::string token;

    f >> token;
    while (f.eof() == false) {
        if (readers.find(token) == readers.end()) throw std::runtime_error("Error parsing SMD");
        readers[token]->read(f);
        f >> token;
    }
    return sr->_frames;
}

skeleton read_skeleton(const char* filename)
{
    LOG_DEBUG << "Reading skeleton from: " << filename;
    std::map<std::string, token_reader*> readers;

    std::unique_ptr<version_reader> vr = std::make_unique<version_reader>();
    std::unique_ptr<nodes_reader> nr = std::make_unique<nodes_reader>();
    std::unique_ptr<skeleton_reader> sr = std::make_unique<skeleton_reader>(nr.get());

    readers["version"] = vr.get();
    readers["nodes"] = nr.get();
    readers["skeleton"] = sr.get();

    std::ifstream f(filename, std::ios::in);
    if (!f) throw std::runtime_error("File not found");

    std::string token;

    f >> token;
    while (f.eof() == false) {
        if (readers.find(token) == readers.end()) break;//throw std::runtime_error("");
        readers[token]->read(f);
        f >> token;
    }
    skeleton ret = nr->_skeleton;
    ret.rest = sr->_frames[0];
    return ret;
}

bool load_png_for_texture(unsigned char** img,
                                 unsigned int* width,
                                 unsigned int* height,
                                 const char* filename)
{
    bool success = false;
    unsigned error = lodepng_decode32_file(img, width, height, filename);
    if (error) {
        std::runtime_error("error png!");
    } else {
        // Flip and invert the image
        unsigned char* imagePtr = *img;
        int halfTheHeightInPixels = *height / 2;
        int heightInPixels = *height;
        // Assume RGBA for 4 components per pixel
        int numColorComponents = 4;
        // Assuming each color component is an unsigned char
        int widthInChars = *width * numColorComponents;
        unsigned char* top = NULL;
        unsigned char* bottom = NULL;
        unsigned char temp = 0;
        for (int h = 0; h < halfTheHeightInPixels; ++h) {
            top = imagePtr + h * widthInChars;
            bottom = imagePtr + (heightInPixels - h - 1) * widthInChars;
            for (int w = 0; w < widthInChars; ++w) {
                // Swap the chars around.
                temp = *top;
                *top = *bottom;
                *bottom = temp;
                ++top;
                ++bottom;
            }
        }
        success = true;
    }
    return success;
}

}
