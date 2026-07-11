#include <iostream>
#include <cmath>
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include "Model.h"

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------

struct Controls {
  bool up    = false;  // W or Up arrow
  bool down  = false;  // S or Down arrow
  bool left  = false;  // A or Left arrow
  bool right = false;  // D or Right arrow
  bool qKey = false;
  bool eKey = false;
};

struct SDLOffscreenBuffer {
  SDL_Surface* surface;
  int          width;
  int          height;
  int          bytes_per_pixel;
};

struct DepthBuffer {
  int width;
  int height;
  std::vector<float> depth;

  DepthBuffer(int width, int height)
      : width(width), height(height), depth(width*height, std::numeric_limits<float>::max()) {};

  float& at(int x, int y)        { return depth[y * width + x]; }
  void clear(float value = std::numeric_limits<float>::max()) {
    depth.resize(width*height, value);
    std::fill(depth.begin(), depth.end(), value);
  }
};

// ---------------------------------------------------------------------------
// Module-level globals (internal linkage)
// ---------------------------------------------------------------------------

namespace {

constexpr char kWindowTitle[] = "Title";
constexpr int  kBytesPerPixel = 4;
constexpr int  kScrollSpeed   = 3;
constexpr float kNear         = -1.0f;
constexpr float kFar          = 1.0f;
constexpr bool showFPS        = true;

int                g_win_width   = 640;
int                g_win_height  = 480;
SDL_Window*        g_game_window = nullptr;
SDLOffscreenBuffer g_back_buffer;
DepthBuffer        g_depth_buffer = {g_win_width, g_win_height};
Controls           g_controls;

}  // namespace

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

void ResizeSurface(SDLOffscreenBuffer* frame_buffer);
bool HandleEvent(const SDL_Event& event);
void InitSdl();
void RenderGradient(SDLOffscreenBuffer* frame_buffer, int x_off, int y_off);
void RunMainLoop();

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

void ResizeSurface(SDLOffscreenBuffer* frame_buffer, DepthBuffer* depth_buffer) {
  if (frame_buffer->surface) {
    SDL_DestroySurface(frame_buffer->surface);
  }
  
  depth_buffer->width           = g_win_width;
  depth_buffer->height          = g_win_height;
  depth_buffer->clear();

  frame_buffer->width           = g_win_width;
  frame_buffer->height          = g_win_height;
  frame_buffer->bytes_per_pixel = kBytesPerPixel;

  frame_buffer->surface = SDL_CreateSurface(g_win_width, g_win_height,
                                            SDL_PIXELFORMAT_ARGB8888);
  if (!frame_buffer->surface) {
    std::cerr << "SDL_CreateSurface failed: " << SDL_GetError() << std::endl;
  }
}

// Returns true if the application should quit.
bool HandleEvent(const SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_QUIT:
      std::cout << "Game window closing..." << std::endl;
      return true;

    case SDL_EVENT_WINDOW_RESIZED:
      std::cout << "Window resized ("
                << event.window.data1 << ", "
                << event.window.data2 << ")" << std::endl;
      g_win_width  = event.window.data1;
      g_win_height = event.window.data2;
      ResizeSurface(&g_back_buffer, &g_depth_buffer);
      break;

    case SDL_EVENT_KEY_DOWN:
      switch (event.key.key) {
        case SDLK_W: case SDLK_UP:    g_controls.up    = true; break;
        case SDLK_S: case SDLK_DOWN:  g_controls.down  = true; break;
        case SDLK_A: case SDLK_LEFT:  g_controls.left  = true; break;
        case SDLK_D: case SDLK_RIGHT: g_controls.right = true; break;
        case SDLK_E: g_controls.eKey = true; break;
        case SDLK_Q: g_controls.qKey = true; break;
        case SDLK_ESCAPE: return true; break;
        default: break;
      }
      break;

    case SDL_EVENT_KEY_UP:
      switch (event.key.key) {
        case SDLK_W: case SDLK_UP:    g_controls.up    = false; break;
        case SDLK_S: case SDLK_DOWN:  g_controls.down  = false; break;
        case SDLK_A: case SDLK_LEFT:  g_controls.left  = false; break;
        case SDLK_D: case SDLK_RIGHT: g_controls.right = false; break;
        case SDLK_E: g_controls.eKey = false; break;
        case SDLK_Q: g_controls.qKey = false; break;
        default: break;
      }
      break;

    default:
      break;
  }
  return false;
}

void InitSdl() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return;
  }

  g_game_window = SDL_CreateWindow(
      kWindowTitle, g_win_width, g_win_height, SDL_WINDOW_RESIZABLE);
  if (!g_game_window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    return;
  }
  std::cout << "Successfully created window." << std::endl;
  // No renderer created — using the surface path instead.
}

void RenderGradient(SDLOffscreenBuffer* frame_buffer, int x_off, int y_off) {
  // surface->pixels is the pixel buffer; surface->pitch is bytes per row.
  auto* pixels = static_cast<uint32_t*>(frame_buffer->surface->pixels);
  int   pitch  = frame_buffer->surface->pitch / kBytesPerPixel;  // pitch in pixels

  for (int y = 0; y < frame_buffer->height; ++y) {
    for (int x = 0; x < frame_buffer->width; ++x) {
      constexpr uint8_t kAlpha = 255;
      constexpr uint8_t kRed   = 0;
      auto green = static_cast<uint8_t>(y + y_off);
      auto blue  = static_cast<uint8_t>(x + x_off);
      pixels[y * pitch + x] =
          (kAlpha << 24) | (kRed << 16) | (green << 8) | blue;
    }
  }
}

void PutPixel(SDL_Surface* screen,int x,int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  if (x < 0 || x >= screen->w || y < 0 || y >= screen->h) return;
  Uint32 pitch_per_pixel = screen->pitch / sizeof(Uint32);
  Uint32* p_screen = (Uint32*)screen->pixels;
  p_screen += y*pitch_per_pixel+x;
  *p_screen = SDL_MapRGBA(SDL_GetPixelFormatDetails(screen->format), NULL, r,g,b,a);  
}

// I created a namespace to hold this stuff since I want to implement a few different versions.
namespace Bresenham {

  // this is really flawed, not only does it use floating point multiplication but it can only draw lines in a small portion of 360deg
  void NaiveDrawLine(SDL_Surface* screen, int x1, int y1, int x2, int y2) {
      float slope = (float)(y2-y1) / (x2-x1);
      float c = y1 - slope * x1;
      for (int x = x1; x <= x2; x++) {
        // Assuming that the round function finds
        // closest integer to a given float.
        int y = round(slope*x + c);
        PutPixel(screen, x, y, 255, 255, 255, 255);
      }
  }

  // notice how there isnt any floating point multiplication here? Multiplication by 2 is a single bitshift! Reducing muiltiplication to adds and shifts durastically improves the performance of the algorith, and is how modern graphcis hardware does it.
  void IntDrawLineSingleDir(SDL_Surface* screen, int x1, int y1, int x2, int y2)
  {
    int dx = (x1-x2);
    int dy = (y1-y2);
    
    if(dx != 0){
      int pixY = y1;
      int decision = 2*dy - dx; // initializing the decision param - gets rid of the need to track a slope.
      for(int i = 0; i<dx+1; i++){
        PutPixel(screen, x1 + i, pixY, 255, 255, 255, 255); // place the current pixel

        // Without this decision param increment, then the line is straigh at x1,y1 -> x2,y1
        if(decision >= 0){
          pixY += 1;
          decision = decision - 2*dx;
        } else {
          decision = decision + 2*dy;
        }
      }
    }
  }
  
  void PutLineHorizontal(SDL_Surface* screen, int x1, int y1, int x2, int y2){
    if(x1 > x2){
      // flip that shit
      int tempX = x1;
      int tempY = y1;
      x1 = x2;
      x2 = tempX;
      y1 = y2;
      y2 = tempY;
    }
    int dx = (x2-x1);
    int dy = (y2-y1);

    int dir = dy < 0 ? -1 : 1;
    dy = dy * dir;
    
    if(dx != 0){
      int pixY = y1;
      int decision = 2*dy - dx; // initializing the decision param - gets rid of the need to track a slope.
      for(int i = 0; i<dx+1; i++){
        PutPixel(screen, x1 + i, pixY, 255, 255, 255, 255); // place the current pixel

        // Without this decision param increment, then the line is straigh at x1,y1 -> x2,y1
        if(decision >= 0){
          pixY += dir;
          decision = decision + 2*dy - 2*dx;
        } else {
          decision = decision + 2*dy;
        }
      }
    }
  }

  void PutLineVertical(SDL_Surface* screen, int x1, int y1, int x2, int y2){
    if(y1 > y2){
      // flip that shit
      int tempX = x1;
      int tempY = y1;
      x1 = x2;
      x2 = tempX;
      y1 = y2;
      y2 = tempY;
    }
    int dx = (x2-x1);
    int dy = (y2-y1);

    int dir = dx < 0 ? -1 : 1;
    dx = dx * dir;
    
    if(dy != 0){
      int pixX = x1;
      int decision = 2*dx - dy; // initializing the decision param - gets rid of the need to track a slope.
      for(int i = 0; i<dy+1; i++){
        PutPixel(screen, pixX, y1 + i, 255, 255, 255, 255); // place the current pixel

        // Without this decision param increment, then the line is straigh at x1,y1 -> x2,y1
        if(decision >= 0){
          pixX += dir;
          decision = decision + 2*dx - 2*dy;
        } else {
          decision = decision + 2*dx;
        }
      }
    }
  }

// final implementation handles all directions 
void PutLine(SDL_Surface* screen, int x1, int y1, int x2, int y2)
{
    bool horizontal = std::abs(x2-x1) > std::abs(y2-y1);
    if(horizontal) {
      PutLineHorizontal(screen, x1, y1, x2, y2);
    } else {
      PutLineVertical(screen, x1, y1, x2, y2);
    }
  }
}

namespace Math {
  struct Matrix {
      int rows, cols;
      std::vector<float> data;
      
      Matrix() : rows(0), cols(0), data(0) {}                          // Matrix mat;
      Matrix(int r, int c) : rows(r), cols(c), data(r * c) {}         // Matrix mat(4, 4);
      Matrix(int r, int c, std::initializer_list<float> values)       // Matrix mat(4,4, {1...
          : rows(r), cols(c), data(values) {}

      // Access element at (row i, col j)
      float at(int i, int j) const {
          return data[i * cols + j];
      }

      float& set(int i, int j, float value) {
          return data[i * cols + j] = value;
      }
  };

  // easy way to init a matrix: Matrix mat = Matrix4x4::Identity();
  Matrix identity() {
      Matrix m(4,4);
      for (int i = 0; i < 4; i++)
          m.set(i, i, 1.0f);
      return m;
  }

  Matrix matMult(const Matrix& firstMat, const Matrix& secondMat){
    int n = firstMat.rows;
    int m = firstMat.cols; // n x m & m x p (m stays the same)
    int p = secondMat.cols;
    Matrix newMat(n,p);

  // Given two matrices, A of n rows and k columns ((n,k) from now on) and a (k,m) 
  // matrix B of, the product of AB is an (n,m) matrix C. The element C[r][c] is 
  // defined as a the dot product of row r of A with the column c of B. 

    // for i in tqdm(range(n)):    
    //    for j in range(n):        
    //       for k in range(n):            
    //          C[i][j] += A[i][k] * B[k][j]

    // for(int i = 0; i < n; ++i) {        
    //    for(int k = 0; k < n; ++k) {      
    //       for(int j = 0; j < n; ++j)     {                
    //          C[i][j] += A[i][k] * B[k][j];            
    //       } 
    //    }    
    // }

    for(int i = 0; i<n; i++){
      for(int j = 0; j<p; j++){
        for(int k = 0; k<m; k++) {
            newMat.set(i,j, newMat.at(i,j) + (firstMat.at(i,k) * secondMat.at(k,j)));
          }
        }
    }

    return newMat;
  }
  
  Matrix matMultSameSize(const Matrix& firstMat, const Matrix& secondMat){
    int n = firstMat.rows;
    Matrix newMat(n,n);
    for(int i = 0; i<n; i++){
      for(int k = 0; k<n; k++) {
        for(int j = 0; j<n; j++){
              newMat.set(i,j, newMat.at(i,j) + (firstMat.at(i,k) * secondMat.at(k,j)));
          }
        }
    }

    return newMat;
  }

  // Some vector stuff too
  Vector3 up()      { return Vector3(0,1,0); }
  Vector3 down()    { return Vector3(0,-1,0); }
  Vector3 left()    { return Vector3(-1,0,0); }
  Vector3 right()   { return Vector3(1,0,0); }
  Vector3 forward() { return Vector3(0, 0, 1);  }
  Vector3 back()    { return Vector3(0, 0, -1); } // we are sticking with a right handed system todo(jack): port this over to left hand for fun?

  float dotProd(Vector3 vec_one, Vector3 vec_two){
    return (vec_one.x * vec_two.x) + (vec_one.y * vec_two.y) + (vec_one.z * vec_two.z);
  }

  // ----------------- Math standard Matricies for affine transformations --------------------

  Matrix makeScale(float sx, float sy, float sz) {
      return Math::Matrix(4, 4, {
          sx,  0,   0,   0,
          0,   sy,  0,   0,
          0,   0,   sz,  0,
          0,   0,   0,   1,
      });
  }

  Matrix makeRotationX(float angle) {
    return Math::Matrix(4, 4, {
        1,  0,           0,            0,
        0,  cosf(angle), -sinf(angle), 0,
        0,  sinf(angle),  cosf(angle), 0,
        0,  0,            0,           1,
    });
  }
  Matrix makeRotationY(float angle) {
    return Math::Matrix(4, 4, {
         cosf(angle),  0,  sinf(angle),  0,
         0,            1,  0,            0,
        -sinf(angle),  0,  cosf(angle),  0,
         0,            0,  0,            1,
    });
  }
  Matrix makeRotationZ(float angle) {
    return Math::Matrix(4, 4, {
        cosf(angle), -sinf(angle),  0,  0,
        sinf(angle),  cosf(angle),  0,  0,
        0,            0,            1,  0,
        0,            0,            0,  1,
    });
  }

  Matrix makeTranslation(float tx, float ty, float tz) {
      return Math::Matrix(4, 4, {
          1,  0,  0,  tx,
          0,  1,  0,  ty,
          0,  0,  1,  tz,
          0,  0,  0,  1,
      });
  }


  // Then we move the model from world space into view or camera space...
  // d is the focal length of the camea todo(jack): understand this AS WELL AS HUMANLY POSSIBLE
  Matrix makeProjection(float cameraNearClip, float cameraFarClip, float d) {
      return Math::Matrix(4, 4, {
          d,  0,  0,  0,
          0,  d,  0,  0,
          0,  0,  (cameraFarClip + cameraNearClip / (cameraNearClip - cameraFarClip)),  (2 * cameraFarClip * cameraNearClip / (cameraNearClip - cameraFarClip)),
          0,  0,  -1,  0,
      });
  }


}


namespace World {

// we need to know the cameras position in worldspace
Vector3 cameraPos        = {0,0,0};
Vector3 cameraRot        = {0,0,0};
float focalLength        = 1.0f; // distance to the viewing plane.
double cameraNearClip    = 3.0f;
double cameraFarClip     = 10.0f;
double rotationAngleX    = 0 * (M_PI / 180.0);
double rotationAngleY    = 0 * (M_PI / 180.0);
double rotationAngleZ    = 0 * (M_PI / 180.0);
Math::Matrix translation = Math::makeTranslation(2,-1,10);
Math::Matrix rotation    = Math::matMult(Math::matMult(Math::makeRotationX(rotationAngleX), \
                           Math::makeRotationY(rotationAngleY)), \
                           Math::makeRotationZ(rotationAngleZ));
Math::Matrix scale       = Math::makeScale(1.5,1,1); 
Math::Matrix model       = Math::matMult(Math::matMult(translation, rotation), scale);
Math::Matrix projection  = Math::makeProjection((float)cameraNearClip, (float)cameraFarClip, focalLength);


// This used to return std::tuple<int,int>

// Project a vec3 in Model/world space into an orthogonal representation in screen space.
Math::Matrix project(Vector3 v, float minX, float maxX, float minY, float maxY) {

      // Then we move the model from world space into view or camera space...
      Math::Matrix view(4,4, {
        Math::right().x, Math::right().y, Math::right().z, -Math::dotProd(Math::right(),cameraPos),
        Math::up().x, Math::up().y, Math::up().z, -Math::dotProd(Math::up(),cameraPos),
        Math::forward().x, Math::forward().y, Math::forward().z, -Math::dotProd(Math::forward(),cameraPos),
        0, 0, 0, 1
      });

      Math::Matrix vecMat(4,1, {
          v.x,
          v.y,
          v.z,
          1
      });
  

      // Math::Matrix viewPos = Math::matMult(Math::matMult(projection, Math::matMult(view, model)), vecMat);    

      // so we basically need to do this instead, return the view space representation.
      return Math::matMult(Math::matMult(projection, Math::matMult(view, model)), vecMat);    
    }
}

std::tuple<int,int> projectViewMat(const Math::Matrix viewPos){
    float viewPosX = viewPos.at(0,0);
    float viewPosY = viewPos.at(1,0);
    float viewPosZ = viewPos.at(2,0);
    float viewPosW = viewPos.at(3,0);

    float px = 0; 
    float py = 0;
    px = viewPosX / viewPosW; // this is called the "perspective divide"
    py = viewPosY / viewPosW;

    int size = std::min(g_win_width, g_win_height)/2; // Keep the projection square

    int x_offset = (g_win_width  - size) / 2; // center the model in the screen
    int y_offset = (g_win_height - size) / 2;

    return { (int)-(px * size) + x_offset,
             (int)-(-py * size) + y_offset };
}

Vector3 crossProduct(Vector3 vect_A, Vector3 vect_B)
{
    Vector3 cross_P = {};
    cross_P.x = vect_A.y * vect_B.z - vect_A.z * vect_B.y;
    cross_P.y = vect_A.z * vect_B.x - vect_A.x * vect_B.z;
    cross_P.z = vect_A.x * vect_B.y - vect_A.y * vect_B.x;
    return cross_P;
}

void createTriangle(int current_index, const unsigned int* indices, const float* verts, float minX, float maxX, float minY, float maxY) {
    int indicesIndex = current_index * 3;
    unsigned int v1 = indices[indicesIndex];
    unsigned int v2 = indices[indicesIndex+1];
    unsigned int v3 = indices[indicesIndex+2];

    float x0 = verts[v1*3+0], y0 = verts[v1*3+1], z0 = verts[v1*3+2];
    float x1 = verts[v2*3+0], y1 = verts[v2*3+1], z1 = verts[v2*3+2];
    float x2 = verts[v3*3+0], y2 = verts[v3*3+1], z2 = verts[v3*3+2];
  
    //auto [ax, ay] = World::project(Vector3{x0,y0,z0}, minX, maxX, minY, maxY);
    //auto [bx, by] = World::project(Vector3{x1,y1,z1}, minX, maxX, minY, maxY);
    //auto [cx, cy] = World::project(Vector3{x2,y2,z2}, minX, maxX, minY, maxY);

    Math::Matrix pointOneView = World::project(Vector3{x0,y0,z0}, minX, maxX, minY, maxY);
    Math::Matrix pointTwoView = World::project(Vector3{x1,y1,z1}, minX, maxX, minY, maxY);
    Math::Matrix pointThreeView = World::project(Vector3{x2,y2,z2}, minX, maxX, minY, maxY); 
    float onePosZ = pointOneView.at(2,0);
    float twoPosZ = pointTwoView.at(2,0);
    float threePosZ = pointThreeView.at(2,0);

    if(onePosZ < World::cameraNearClip || twoPosZ < World::cameraNearClip || threePosZ < World::cameraNearClip) {
      return;
    }

    auto [ax, ay] = projectViewMat(pointOneView); 
    auto [bx, by] = projectViewMat(pointTwoView);
    auto [cx, cy] = projectViewMat(pointThreeView);

    int min_tri_x = std::min(ax, std::min(bx, cx));
    int max_tri_x = std::max(ax, std::max(bx, cx));
    int min_tri_y = std::min(ay, std::min(by, cy));
    int max_tri_y = std::max(ay, std::max(by, cy));

    // Rasterize!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Edge Vectors
    Vector3 ab = { (float)(bx - ax), (float)(by - ay), 0 };
    Vector3 bc = { (float)(cx - bx), (float)(cy - by), 0 };
    Vector3 ca = { (float)(ax - cx), (float)(ay - cy), 0 };

    // interate over ever candidate pixel (winding order clockwise bc wtf opengl)
    for(int i = min_tri_x; i <= max_tri_x; i++){
      for(int j = min_tri_y; j <= max_tri_y; j++){
        // We get a vector for each vertex pointing to the point we are on currently
        Vector3 ap = { (float)(i - ax), (float)(j - ay), 0 };
        Vector3 bp = { (float)(i - bx), (float)(j - by), 0 };
        Vector3 cp = { (float)(i - cx), (float)(j - cy), 0 };

        // if we are to the right of every vector
        //isright = is cross product positive or negative

        bool is_right = true; 
        float crossW0 = 0.0f; 
        float crossW1 = 0.0f;
        float crossW2 = 0.0f; 

        crossW0 = crossProduct(bc, bp).z; // weight for vertex A
        crossW1 = crossProduct(ca, cp).z; // weight for vertex B  
        crossW2 = crossProduct(ab, ap).z; // weight for vertex C
        float total_area = crossW0 + crossW1 + crossW2;

        // We are to the "left" of atleast one of the edges
        if(crossW0 < 0 || crossW1 < 0 || crossW2 < 0){
          continue;
        }

        crossW0 /= total_area;
        crossW1 /= total_area;
        crossW2 /= total_area;

        // We need to get our normalized z-value for depth buffering
        // right now we can just get an average of the z coordinates of all our verticies...
        // ... then we can multiply each of that with the barycentric percentage for the given point
        float newDepthValue = (z0 * crossW0) + (z1 * crossW1) + (z2 * crossW2);
        float clampedDepthValue = (kFar - newDepthValue) / (kFar - kNear);
        if(i < 0 || i >= g_win_width || j < 0 || j >= g_win_height) continue; // todo(jack): actually learn what this is...
        float currentDepthValue = g_depth_buffer.at(i,j);

        // At this point we know they are all to the right
        if(newDepthValue < currentDepthValue){
          g_depth_buffer.at(i,j) = newDepthValue;
          int shade = (int)(clampedDepthValue * 255);
          PutPixel(g_back_buffer.surface, i, j, shade, shade, shade, 255); // we paint with the barycentric coordinates
        }
      }
    }

    // Bresenham::PutLine(g_back_buffer.surface, ax, ay, bx, by);
    // Bresenham::PutLine(g_back_buffer.surface, bx, by, cx, cy);
    // Bresenham::PutLine(g_back_buffer.surface, cx, cy, ax, ay);
}

void drawModel(const Model& newModel) {
    const unsigned int* indices = newModel.getIndices();
    const float* verts = newModel.getVertices();

    int rotateDegrees = 0;
    float minX=1e9, maxX=-1e9, minY=1e9, maxY=-1e9;
    for(int i = 0; i < (int)newModel.getVertexCount(); i++) {
        //float currentX = verts[i*3+0];
        //float currentY = verts[i*3+1];
        minX = std::min(minX, verts[i*3+0]);
        maxX = std::max(maxX, verts[i*3+0]);
        minY = std::min(minY, verts[i*3+1]);
        maxY = std::max(maxY, verts[i*3+1]);
        
        // reset X with rotated version
        //verts[i*3+0] = (currentX * std::cos(rotateDegrees)) - (currentY * std::sin(rotateDegrees));
        //verts[i*3+1] = (currentX * std::sin(rotateDegrees)) + (currentY * std::cos(rotateDegrees));
    }

    for(int i = 0; i < (int)newModel.getTriangleCount(); i++) {
        // Creating each triangle here...
        createTriangle(i, indices, verts, minX, maxX, minY, maxY);
    }
}

void RunMainLoop() {
  ResizeSurface(&g_back_buffer, &g_depth_buffer);
  int  x_off   = 0;
  int  y_off   = 0;
  bool running = true;

  Model newModel;
  newModel.read("../assets/diablo.obj");
  newModel.printSelf(); 

  const unsigned int* indices = newModel.getIndices();

  Uint64 perf_count_freq = SDL_GetPerformanceFrequency();
  Uint64 last_counter    = SDL_GetPerformanceCounter();
  Uint64 start_time      = SDL_GetPerformanceCounter();

  while (running) {
    Uint64 end_counter     = SDL_GetPerformanceCounter();
    Uint64 counter_elapsed = end_counter - last_counter;
    Uint64 total_time      = end_counter - start_time;
    float  ms_per_frame    = (1000.0f * (float)counter_elapsed) / (float)perf_count_freq;
    float  fps             = (float)perf_count_freq / (float)counter_elapsed;
    
    if(showFPS)
    {
      std::cout << "FPS: " << fps << " | ms/f: " << ms_per_frame
              << " | TotalTime: " << total_time << std::endl;
    }

    last_counter = end_counter;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (HandleEvent(event)) {
        running = false;
      }
    }

    if (g_controls.up){
      World::cameraPos.z += 0.1;
      y_off -= kScrollSpeed;
    }
    if (g_controls.down){
      World::cameraPos.z -= 0.1;
      y_off += kScrollSpeed;
    }
    if (g_controls.left){ 
      World::cameraPos.x -= 0.1;
      x_off -= kScrollSpeed;
    }
    if (g_controls.right){
      World::cameraPos.x += 0.1;
      x_off += kScrollSpeed;
    }
    if (g_controls.eKey){ 
      World::cameraPos.y -= 0.1;
    }
    if (g_controls.qKey){
      World::cameraPos.y += 0.1;
    }
    
    SDL_ClearSurface(g_back_buffer.surface, 0, 0, 0, 1);
    g_depth_buffer.clear(std::numeric_limits<float>::max());

    RenderGradient(&g_back_buffer, x_off, y_off);
    drawModel(newModel);

    // Blit our offscreen surface to the window surface, then present.
    SDL_Surface* window_surface = SDL_GetWindowSurface(g_game_window);
    if(window_surface) {
      SDL_BlitSurface(g_back_buffer.surface, nullptr, window_surface, nullptr);
      SDL_UpdateWindowSurface(g_game_window);
    }
  }
}

int main(int argc, char* argv[]) {
  InitSdl();
  RunMainLoop();
  return 0;
}
