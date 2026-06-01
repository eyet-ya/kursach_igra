#pragma once

#include <list>
#include <windows.h>
#include <GL/gl.h>
#include <map>
#include <string>
#include <vector>

struct ObjVertex {
    double x = 0, y = 0, z = 0, w = 1;
    inline double* _ptr() { return reinterpret_cast<double*>(this); }
};

struct ObjTexCord {
    double u = 0, v = 0, w = 1;
    inline double* _ptr() { return reinterpret_cast<double*>(this); }
};

struct ObjNormal {
    double x = 0, y = 0, z = 0;
    inline double* _ptr() { return reinterpret_cast<double*>(this); }
};

struct ObjFace {
    std::list<ObjVertex> vertex;
    std::list<ObjTexCord> texCoord;
    std::list<ObjNormal> normal;
    std::string materialName;
};

class ObjModel {
private:
    std::list<ObjFace> Faces;
    std::map<std::string, GLuint> materialTextures;
    GLuint listId = -1;

public:
    ObjModel() {}
    ~ObjModel() {
        if (listId != -1) glDeleteLists(listId, 1);
        for (auto& tex : materialTextures) {
            glDeleteTextures(1, &tex.second);
        }
    }

    int LoadModel(const char* filename);
    void SetMaterialTexture(const std::string& materialName, GLuint texId);
    void LoadAndSetTexture(const std::string& materialName, const std::string& texturePath);
    void BuildDisplayList();
    void Draw();
};
