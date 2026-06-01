#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include "stb_image.h"
    
static GLuint LoadTextureFromFile(const std::string& filename) {
    int x, y, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 4);

    if (!data) {
        return 0;
    }

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    return texId;
}

int ObjModel::LoadModel(const char* filename) {
    std::vector<ObjVertex> V;
    std::vector<ObjTexCord> VT;
    std::vector<ObjNormal> VN;
    std::string currentMaterial;

    std::ifstream fin(filename);
    if (!fin.is_open()) {
        return 0;
    }

    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream isstr(line);
        std::string mode;
        isstr >> mode;

        if (mode == "v") {
            ObjVertex v;
            isstr >> v.x >> v.y >> v.z;
            V.push_back(v);
        }
        else if (mode == "vt") {
            ObjTexCord t;
            isstr >> t.u >> t.v;
            VT.push_back(t);
        }
        else if (mode == "vn") {
            ObjNormal n;
            isstr >> n.x >> n.y >> n.z;
            VN.push_back(n);
        }
        else if (mode == "usemtl") {
            isstr >> currentMaterial;
        }
        else if (mode == "f") {
            ObjFace f;
            f.materialName = currentMaterial;

            std::string face;
            while (isstr >> face) {
                std::istringstream faceStream(face);
                std::string digit;
                int vIdx = 0, tIdx = 0, nIdx = 0;
                int part = 0;

                while (std::getline(faceStream, digit, '/')) {
                    if (part == 0 && !digit.empty()) vIdx = std::stoi(digit);
                    if (part == 1 && !digit.empty()) tIdx = std::stoi(digit);
                    if (part == 2 && !digit.empty()) nIdx = std::stoi(digit);
                    part++;
                }

                if (vIdx > 0 && vIdx <= (int)V.size()) f.vertex.push_back(V[vIdx - 1]);
                if (tIdx > 0 && tIdx <= (int)VT.size()) f.texCoord.push_back(VT[tIdx - 1]);
                if (nIdx > 0 && nIdx <= (int)VN.size()) f.normal.push_back(VN[nIdx - 1]);
            }
            Faces.push_back(f);
        }
    }
    fin.close();

    return 1;
}

void ObjModel::SetMaterialTexture(const std::string& materialName, GLuint texId) {
    materialTextures[materialName] = texId;
}

void ObjModel::LoadAndSetTexture(const std::string& materialName, const std::string& texturePath) {
    GLuint texId = LoadTextureFromFile(texturePath);
    if (texId != 0) {
        materialTextures[materialName] = texId;
    }
}

void ObjModel::BuildDisplayList() {
    if (listId != -1) glDeleteLists(listId, 1);
    listId = glGenLists(1);

    glNewList(listId, GL_COMPILE);

    std::string lastMaterial;
    GLuint currentTexId = 0;

    for (auto& face : Faces) {
        if (face.materialName != lastMaterial) {
            auto it = materialTextures.find(face.materialName);
            if (it != materialTextures.end() && it->second != 0) {
                if (currentTexId != it->second) {
                    glBindTexture(GL_TEXTURE_2D, it->second);
                    glEnable(GL_TEXTURE_2D);
                    currentTexId = it->second;
                }
            }
            else {
                if (currentTexId != 0) {
                    glDisable(GL_TEXTURE_2D);
                    currentTexId = 0;
                }
            }
            lastMaterial = face.materialName;
        }

        glBegin(GL_POLYGON);

        auto it_n = face.normal.begin();
        auto it_t = face.texCoord.begin();

        for (auto it_v = face.vertex.begin(); it_v != face.vertex.end(); ++it_v) {
            if (it_n != face.normal.end()) {
                glNormal3dv((it_n++)->_ptr());
            }
            if (it_t != face.texCoord.end()) {
                glTexCoord2dv((it_t++)->_ptr());
            }
            glVertex3dv(it_v->_ptr());
        }
        glEnd();
    }

    glEndList();
}

void ObjModel::Draw() {
    glCallList(listId);
}