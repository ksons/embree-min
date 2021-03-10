#include <embree3/rtcore.h>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Vertex
{
    float x, y, z;
};
struct Triangle
{
    int v0, v1, v2;
};
struct Pixel
{
    unsigned char r, g, b;
};

struct Color
{
    float r, g, b;
};

void addTriangleMesh(RTCDevice device, RTCScene scene)
{
    size_t numTriangles = 1;
    size_t numVertices = 3;

    // create geometry (like alloc() in C?)
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    // fill vertex buffer
    Vertex *vertices = (Vertex *)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex), 8);
    vertices[0].x = -1;
    vertices[0].y = -1;
    vertices[0].z = -1;

    vertices[1].x = -1;
    vertices[1].y = -1;
    vertices[1].z = +1;

    vertices[2].x = -1;
    vertices[2].y = +1;
    vertices[2].z = -1;

    vertices[3].x = -1;
    vertices[3].y = +1;
    vertices[3].z = +1;

    vertices[4].x = +1;
    vertices[4].y = -1;
    vertices[4].z = -1;

    vertices[5].x = +1;
    vertices[5].y = -1;
    vertices[5].z = +1;

    vertices[6].x = +1;
    vertices[6].y = +1;
    vertices[6].z = -1;

    vertices[7].x = +1;
    vertices[7].y = +1;
    vertices[7].z = +1;

    int tri = 0;
    Triangle *triangles = (Triangle *)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 12);

    // left side
    triangles[tri].v0 = 0;
    triangles[tri].v1 = 1;
    triangles[tri].v2 = 2;
    tri++;
    triangles[tri].v0 = 1;
    triangles[tri].v1 = 3;
    triangles[tri].v2 = 2;
    tri++;

    // right side
    triangles[tri].v0 = 4;
    triangles[tri].v1 = 6;
    triangles[tri].v2 = 5;
    tri++;
    triangles[tri].v0 = 5;
    triangles[tri].v1 = 6;
    triangles[tri].v2 = 7;
    tri++;

    // bottom side
    triangles[tri].v0 = 0;
    triangles[tri].v1 = 4;
    triangles[tri].v2 = 1;
    tri++;
    triangles[tri].v0 = 1;
    triangles[tri].v1 = 4;
    triangles[tri].v2 = 5;
    tri++;

    // top side
    triangles[tri].v0 = 2;
    triangles[tri].v1 = 3;
    triangles[tri].v2 = 6;
    tri++;
    triangles[tri].v0 = 3;
    triangles[tri].v1 = 7;
    triangles[tri].v2 = 6;
    tri++;

    // front side
    triangles[tri].v0 = 0;
    triangles[tri].v1 = 2;
    triangles[tri].v2 = 4;
    tri++;
    triangles[tri].v0 = 2;
    triangles[tri].v1 = 6;
    triangles[tri].v2 = 4;
    tri++;

    // back side
    triangles[tri].v0 = 1;
    triangles[tri].v1 = 5;
    triangles[tri].v2 = 3;
    tri++;
    triangles[tri].v0 = 3;
    triangles[tri].v1 = 5;
    triangles[tri].v2 = 7;
    tri++;

    // commit changes of the scene
    rtcCommitGeometry(geom);
    unsigned int geomID = rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);
}

Color renderPixel(const int x, const int y, int width, int height, RTCScene scene)
{

    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    // initialize ray
    RTCRayHit rh;
    
    rh.ray.org_x = 0.0f; // camera original position
    rh.ray.org_y = 0.0f;
    rh.ray.org_z = -3.5f;

    rh.ray.dir_x = (float)(x - width * 0.5)/256.0;
    rh.ray.dir_y = (float)(y - height * 0.5)/256.0;
    rh.ray.dir_z = 1.0f; // should rewrite

    rh.ray.tnear = 0.0f;
    rh.ray.tfar = std::numeric_limits<float>::infinity();
    rh.ray.time = 0;
    rh.ray.mask = -1;

    rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rh.hit.primID = RTC_INVALID_GEOMETRY_ID;

    // intersect ray
    rtcIntersect1(scene, &context, &rh);

    // shade this pixel
    Color color;
    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 1.0f;
   
    if (rh.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        color.r = 0.0f;
        color.g = 1.0f;
        color.b = 0.0f;
    }

    return color;
}

void writeImage(const char *name, size_t width, size_t height, Pixel *img)
{
    if (stbi_write_png(name, width, height, 3, img, width*3))
    {
        std::cout << "Wrote file '" << name << "' successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to write file '" << name << "'." << std::endl;
    }
}

void renderImage(size_t width, size_t height, RTCScene scene)
{
    Pixel *img = new Pixel[width * height];

    // for each pixel
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Color color = renderPixel(x, y, width, height, scene);
            img[y * width + x].r = (unsigned char)(255.0f * color.r);
            img[y * width + x].g = (unsigned char)(255.0f * color.g);
            img[y * width + x].b = (unsigned char)(255.0f * color.b);
        }
    }

    // output image
    writeImage("out.png", width, height, img);

    delete[](img);
}

int main(int argc, char *argv[])
{
    const size_t width = 256;
    const size_t height = 256;
   
    // initialize at application startup
    RTCDevice device = rtcNewDevice("threads=1,isa=avx,verbose=3");
   
    RTCScene scene = rtcNewScene(device);
   
    addTriangleMesh(device, scene);
    rtcCommitScene(scene);

    renderImage(width, height, scene);

    rtcReleaseScene(scene);
    rtcReleaseDevice(device);
}