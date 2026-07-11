#include "Model.h"
#include <iostream>
#include <fstream>
#include <algorithm>

// PUBLIC

Model::Model()
{
}

Model::~Model()
{
}

void Model::printSelf() const 
{
    std::cout << "===== ObjModel =====\n"
              << "Triangle Count: " << getTriangleCount() << "\n"
              << "   Index Count: " << getIndexCount() << "\n"
              << "  Vertex Count: " << getVertexCount() << "\n"
              << "  Normal Count: " << getNormalCount() << "\n"
              << "TexCoord Count: " << getTexCoordCount() << std::endl;
    std::cout << std::endl;
}

// PRIVATE

void Model::init()
{
    // flush(deallocate) the previous memory
    std::vector<float>().swap(vertices);
    std::vector<float>().swap(normals);
    std::vector<float>().swap(texCoords);

    std::vector<unsigned int>().swap(indices);
    std::vector<Vector3>().swap(faceNormals);

    std::vector<float>().swap(vertexLookup);    // for "v"
    std::vector<float>().swap(normalLookup);    // for "vn"
    std::vector<float>().swap(texCoordLookup);  // for "vt"
    std::vector<unsigned int>().swap(indiciesLookup);

}

///////////////////////////////////////////////////////////////////////////////
// load obj file from file
///////////////////////////////////////////////////////////////////////////////
bool Model::read(const std::string path)
{
    // open an OBJ file
    std::ifstream inFile;
    inFile.open(path.c_str(), std::ios::in);
    if(!inFile.good())
    {
        errorMessage = "Failed to open a OBJ file to read: ";
        errorMessage += path;
        inFile.close();
        return false;
    }

    // get lines of obj file
    std::vector<std::string> vLines;    // "v" lines from obj file
    std::vector<std::string> vnLines;   // "vn" lines from obj file
    std::vector<std::string> vtLines;   // "vt" lines from obj file
    std::vector<std::string> fLines;    // "f" lines from obj file, other lines as well

    for(std::string line; std::getline(inFile, line);)
    {

        while(!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();

        if(line.size() < 2) // skip invalid lines (must have 2 chars per line)
            continue;

        if(line[0] == '#') // skip comment lines begin with #
            continue;

        if(line[0] == 'v')
        {
            if(line[1] == 'n')          // vn
                vnLines.push_back(line);
            else if(line[1] == 't')     // vt
                vtLines.push_back(line);
            else if(line[1] == ' ')     // v
                vLines.push_back(line);
        }
        if(line[0] == 'f')
        {
            fLines.push_back(line);
        }
    }

    std::cout << "vLines size: " << vLines.size() << std::endl;
    std::cout << "vtLines size: " << vtLines.size() << std::endl;
    std::cout << "vnLines size: " << vnLines.size() << std::endl;

    // close opened file
    inFile.close();

    // init arrays
    std::vector<unsigned int>().swap(indices);
    std::vector<Vector3>().swap(faceNormals);
    std::vector<float>().swap(vertices);            
    std::vector<float>().swap(normals);
    std::vector<float>().swap(texCoords);
    indices.reserve(fLines.size() * 3);             // assume they are triang
    faceNormals.reserve(fLines.size());             // normals per face
    vertices.reserve(vLines.size() * 3);
    normals.reserve(vLines.size() * 3);
    if(vtLines.size() > 0)
        texCoords.reserve(vLines.size() * 2);

    // parse "v" lines to vertexLookup
    std::vector<float>().swap(vertexLookup);
    vertexLookup.reserve(vLines.size() * 3);        // x,y,z
    parseVertexLookup(vLines);
    std::vector<std::string>().swap(vLines);        // dealloc memory

    // parse "vn" lines to normalLookup
    std::vector<float>().swap(normalLookup);
    normalLookup.reserve(vnLines.size() * 3);       // nx,ny,nz
    parseNormalLookup(vnLines);
    std::vector<std::string>().swap(vnLines);       // dealloc memory

    // parse "vt" lines to texCoordLookup
    if(vtLines.size() > 0)
    {
        std::vector<float>().swap(texCoordLookup);
        texCoordLookup.reserve(vtLines.size() * 2); // u,v
        parseTexCoordLookup(vtLines);
        std::vector<std::string>().swap(vtLines);   // dealloc memory
    }

    // parse "f" lines with other lines as well: "g", "usemtl", "mtllib"
    parseFaces(fLines);
    std::vector<std::string>().swap(fLines);        // dealloc memory
    std::cout << "vertexLookup size: " << vertexLookup.size() << std::endl;
    vertices = vertexLookup;
    indices = indiciesLookup;

    // clear lookups
    std::vector<float>().swap(vertexLookup);
    std::vector<float>().swap(normalLookup);
    std::vector<float>().swap(texCoordLookup);
    std::vector<unsigned int>().swap(indiciesLookup);

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// parse only "v" lines from obj file
// The vertex positions will be stored in vertexLookup array.
///////////////////////////////////////////////////////////////////////////////
void Model::parseVertexLookup(const std::vector<std::string>& lines)
{
    unsigned int count = (unsigned int)lines.size();
    for(unsigned int i = 0; i < count; i++){
        std::vector<std::string> tokens = {};
        Model::splitString(lines[i], ' ', tokens);
        tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
            [](const std::string& s){ return s.empty(); }), tokens.end());
        vertexLookup.push_back((float)atof(tokens[1].c_str())); // x
        vertexLookup.push_back((float)atof(tokens[2].c_str())); // y
        vertexLookup.push_back((float)atof(tokens[3].c_str())); // z
    }
}



///////////////////////////////////////////////////////////////////////////////
// parse only "vn" lines from obj file
// The vertex normals will be stored in normalLookup array.
///////////////////////////////////////////////////////////////////////////////
void Model::parseNormalLookup(const std::vector<std::string>& lines)
{

}



///////////////////////////////////////////////////////////////////////////////
// parse only "vt" lines from obj file
// The texture coordinates will be stored in texCoordLookup array.
///////////////////////////////////////////////////////////////////////////////
void Model::parseTexCoordLookup(const std::vector<std::string>& lines)
{
   /* 
    Tokenizer tokenizer;

    // convert as float then store to vertexLookup
    unsigned int count = (unsigned int)lines.size();
    for(unsigned int i = 0; i < count; ++i)
    {
        tokenizer.set(lines[i], " ");
        tokenizer.next(); // skip first "vt" token

        // OpenGL uses bottom-left origin, and OBJ is top-left origin
        // invert v coord to OpenGL orientation
        texCoordLookup.push_back((float)atof(tokenizer.next().c_str()));        // u
        texCoordLookup.push_back(1.0f - (float)atof(tokenizer.next().c_str())); // v
    }
  */
}



///////////////////////////////////////////////////////////////////////////////
// parse faces from obj file, also read "g", "usemtl", "mtllib" lines as well
// NOTE: "f" elements should be listed inside a group(g). If face elements come
// before a "g" tag, create a default group and assign the faces into the
// default group.
///////////////////////////////////////////////////////////////////////////////
void Model::parseFaces(const std::vector<std::string>& lines)
{
    for(const std::string& line : lines)
    {
        std::vector<std::string> tokens = {};
        splitString(line, ' ', tokens);
        tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
            [](const std::string& s){ return s.empty(); }), tokens.end());

        auto pushVertex = [&](const std::string& token) {
            std::vector<std::string> values = {};
            splitString(token, '/', values);
            values.erase(std::remove_if(values.begin(), values.end(),
                [](const std::string& s){ return s.empty(); }), values.end());
            int vi = atoi(values[0].c_str()) - 1;
            if(vi < 0 || vi >= (int)(vertexLookup.size() / 3)) {
                std::cout << "BAD INDEX: " << vi << " from token: " << token << std::endl;
                return;
            }
            indiciesLookup.push_back((unsigned int)vi);
        };

        if(tokens.size() >= 4) {
            for(int j = 1; j <= (int)tokens.size() - 3; j++) {
                pushVertex(tokens[1]);      // anchor
                pushVertex(tokens[j + 1]);  // next
                pushVertex(tokens[j + 2]);  // next+1
            }
        } else if(tokens.size() == 5) {
            pushVertex(tokens[1]);
            pushVertex(tokens[2]);
            pushVertex(tokens[3]);
            pushVertex(tokens[1]);
            pushVertex(tokens[3]);
            pushVertex(tokens[4]);
        }
    }
}

void Model::splitString(const std::string& s, char c, std::vector<std::string>& v) {
    std::string::size_type i = 0;
    std::string::size_type j = s.find(c);

   while (j != std::string::npos) {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);
   }
    v.push_back(s.substr(i, s.length())); // always push the last token
}
