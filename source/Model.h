#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <map>
#include <sstream>

// Vector 3 Representation
struct Vector3 {
    float x,y,z;
};

struct Vector2 {
    float x,y;
};

///////////////////////////////////////////////////////////////////////////////
class Model
{
public:
    Model();
    ~Model();

    // load obj file
    bool read(const std::string filename);

    // vertex attributes
    unsigned int getVertexCount() const         { return (unsigned int)vertices.size() / 3; }
    unsigned int getNormalCount() const         { return (unsigned int)normals.size() / 3; }
    unsigned int getTexCoordCount() const       { return (unsigned int)texCoords.size() / 2; }
    unsigned int getIndexCount() const          { return (unsigned int)indices.size(); }  // total
    unsigned int getTriangleCount() const       { return (unsigned int)indices.size()/3; }

    // return data as 1D array
    const float* getVertices() const            { return &vertices[0];  }
    const unsigned int* getIndices() const      { return &indices[0];   }
    const float* getNormals() const             { return &normals[0];   }
    const float* getTexCoords() const           { return &texCoords[0]; }

    const std::string& getErrorMessage() const  { return errorMessage; }
    void printSelf() const;

private:
    void init();
    void parseVertexLookup(const std::vector<std::string>& lines);      // parse "v" lines
    void parseNormalLookup(const std::vector<std::string>& lines);      // parse "vn" lines
    void parseTexCoordLookup(const std::vector<std::string>& lines);    // parse "vt" lines
    void parseFaces(const std::vector<std::string>& lines);             // parse "f" lines and other tags
    void parseMesh(const std::vector<std::string>& lines);              // old parser
    void splitString(const std::string& s, char c, std::vector<std::string>& v);

    std::vector<float> vertices;  // vertex position array for opengl
    std::vector<float> normals;  // vertex normal array for opengl
    std::vector<float> texCoords;  // tex-ccord array for opengl
    std::vector<unsigned int> indices;  // index array for opengl
    std::vector<Vector3> faceNormals; // normals per face

    // split vertex data without sharing vertices for smoothing normals
    std::vector<Vector3> splitVertices;
    std::vector<Vector3> splitNormals;
    std::vector<Vector2> splitTexCoords;
    std::multimap<Vector3, unsigned int> splitVertexMap;
    std::map<unsigned int, unsigned int> sharedVertexLookup;

    // temporary lookup buffers
    std::vector<float> vertexLookup;            // for "v" lines
    std::vector<float> normalLookup;            // for "vn" lines
    std::vector<float> texCoordLookup;          // for "vt" lines
    std::vector<unsigned int> indiciesLookup;  // for "f" lines, map list

    std::string errorMessage;
};

#endif // OBJ_MODEL_H

/*
class Model {
    std::vector<vec3> verts = {};    // array of vertices
    std::vector<int> facet_vrt = {}; // per-triangle index in the above array
public:
    Model(const std::string filename);
    int nverts() const; // number of vertices
    int nfaces() const; // number of triangles
    vec3 vert(const int i) const;                          // 0 <= i < nverts()
    vec3 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
};
*/
