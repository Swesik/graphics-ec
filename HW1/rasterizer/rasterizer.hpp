#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <cstdint>
#include <vector>

#include <_types/_uint32_t.h>

#include "entities.hpp"
#include "image.hpp"
#include "loader.hpp"

class Rasterizer {
public:
    struct gBufferStruct {
        glm::vec3 norm;
        glm::vec3 pos;
        Color texel;
    };

    Rasterizer(Loader& loader);

    /// rasterizer.cpp
    // Render a single triangle, with no transformations, and possible anti-aliasing, based on config
    void DrawPrimitiveRaw(Image& image, Triangle trig, AntiAliasConfig config, uint32_t spp);

    // Add a model to the rasterizer. Provide rotation part of the transformation, and dispatch to the impl version
    void AddModel(MeshTransform transform);

    // Initialize the MSAAMask with the default value specified in impl
    void InitMSSAMask(ImageGrey& MSSAMask, uint32_t num_samples);

    // Initialize the ZBuffer with the default value specified in impl
    void InitZBuffer(ImageGrey& ZBuffer);

    // Initialize the ZBuffer with the default value specified in impl
    void InitGBuffer(ImageBuffer<gBufferStruct>& GBuffer);

    // Render the depth information of a single triangle.
    void DrawPrimitiveDepth(Triangle transformed, Triangle original, ImageGrey& ZBuffer);

    // update the GBuffer with a single triangle
    void DrawPrimitiveGBuffer(Triangle transformed, Triangle original, ImageBuffer<gBufferStruct>& gBuffer);

    // Render a single triangle, with blinn-phong shading
    void DrawPrimitiveShaded(Triangle transformed, Triangle original, Image& image);

    // Render the full image, with blinn-phong shading (via deferred shading)
    void DrawPrimitiveShaded(Image& image);

    // rasterizer_impl.cpp

    /**
     * Given a single pixel in the screen space with the triangle in which the pixel is considered, determine the output
     * color that should be rendered for the pixel. This function will be called for every pixel in the bounding box of
     * the triangle.
     * @param x: x coordinate of the pixel
     * @param y: y coordinate of the pixel
     * @param trig: the triangle in which the pixel is considered; see class `Triangle` in `entities.hpp`
     * @param config: the anti-aliasing configuration, which can be either `NONE` or `SSAA`
     * @param spp: the number of samples per pixel. Only useful if config is set to `SSAA`
     * @param image: the image to render the pixel on. See class `Image` in `image.hpp` for APIs of read/write
     * operations
     * @param color: the color to render the pixel with, if the pixel is completely inside the triangle
     */
    void DrawPixel(uint32_t x, uint32_t y, Triangle trig, AntiAliasConfig config, uint32_t spp, Image& image,
                   Color color);


    /**
     * Add the corresponding model transformation to the rasterizer.
     * @param transform: the transformation to apply to the model. See class `MeshTransform` in `entities.hpp` for
     * detailed definitions.
     * @param rotation: the rotation part of the transformation
     */
    void AddModel(MeshTransform transform, glm::mat4 rotation);

    /**
     * Set the view transformation matrix. This function does not take any argument as `Rasterizer` contains the camera
     * information in the `loader` member.
     */
    void SetView();

    /**
     * Set the projection transformation matrix. This function does not take any argument as `Rasterizer` contains the
     * camera information in the `loader` member.
     */
    void SetProjection();

    /**
     * Set the projection transformation matrix. This function does not take any argument as `Rasterizer` contains the
     * camera information in the `loader` member.
     */
    void SetScreenSpace();

    /**
     * Given a 2D position and a triangle, compute the barycentric coordinates of the position with respect to the
     * triangle. REQUIRE the `pos` fields of the triangle has `z` value 0 (i.e. it is guaranteed that the triangle is in
     * same plane, and is inside the triangle)
     * @param pos: the 2D position to compute the barycentric coordinates for
     * @param trig: the triangle to compute the barycentric coordinates with respect to
     * @return: the barycentric coordinates of the position with respect to the triangle
     */
    glm::vec3 BarycentricCoordinate(glm::vec2 pos, Triangle trig);

    /**
     * Update the depth information at a single pixel in the ZBuffer. This function will be called for every pixel in
     * the bounding box of the triangle.
     * @param x: x coordinate of the pixel
     * @param y: y coordinate of the pixel
     * @param original: the original triangle in the model space (before MVP transformation)
     * @param transformed: the transformed triangle in the screen space (after MVP transformation)
     * @param ZBuffer: the ZBuffer to update the depth information in. See spec, or class `Image` in `image.hpp` for
     * APIs of read/write operations
     */
    void UpdateDepthAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, ImageGrey& ZBuffer);

    /**
     * Update the sample information at a single pixel in the MSSABuffer. This function will be called for every pixel
     * in the bounding box of the triangle.
     * @param x: x coordinate of the pixel
     * @param y: y coordinate of the pixel
     * @param original: the original triangle in the model space (before MVP transformation)
     * @param transformed: the transformed triangle in the screen space (after MVP transformation)
     * @param MSAAMask: the MSAAMask to update the coverage information in. See spec, or class `Image` in `image.hpp`
     * for APIs of read/write operations
     */
    void UpdateMSAAAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, ImageGrey& MSAAMask);

    /**
     * @brief Create a vector MipMap levels. This will modify the this->mipmap_vector
     * Note: this->mipmap_vector[0] cooresponds to the heighest resolution
     *
     * @param texture_filename: filename of the texture
     */
    void CreateMipMap(const std::string& texture_filename);

    /**
     * @brief Get the Texel color from
     *
     * @param tex_coord coordinate on the range [0,1] of the texel
     * @param depth of pixel which is used to determien the mipmap level
     * @return Color of the texel
     */
    Color GetTexel(glm::vec2 tex_coord, float depth);

    /**
     * Update the gbuffer information at a single pixel in the ZBuffer. This function will be called for every pixel in
     * the bounding box of the triangle. This is specifically used for deferred shading.
     * @param x: x coordinate of the pixel
     * @param y: y coordinate of the pixel
     * @param original: the original triangle in the model space (before MVP transformation)
     * @param transformed: the transformed triangle in the screen space (after MVP transformation)
     * @param NormBuffer: the NormBuffer to update the normal information in. See spec, or class `Image` in `image.hpp`
     * for APIs of read/write operations
     */
    void UpdateGBufferAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                              ImageBuffer<gBufferStruct>& gBuffer);

    /**
     * Shade the pixel at the given position, using Blinn-Phong shading model. This function will be called for every
     * pixel This function should only be used with deferred shading
     * @param x: x coordinate of the pixel
     * @param y: y coordinate of the pixel
     * @param image: the image to render the pixel on. See spec, or class `Image` in `image.hpp` for APIs of read/write
     * operations
     */
    void ShadeAtPixel(uint32_t x, uint32_t y, Image& image);

    /**
     * Shade the pixel at the given position, using Blinn-Phong shading model. This function will be called for every
     * pixel in the bounding box of the triangle.
     * @param x: x coordinate of the pixel
     * @param y: y coordinate of the pixel
     * @param original: the original triangle in the model space (before MVP transformation)
     * @param transformed: the transformed triangle in the screen space (after MVP transformation)
     * @param image: the image to render the pixel on. See spec, or class `Image` in `image.hpp` for APIs of read/write
     * operations
     */
    void ShadeAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, Image& image);

public:
    // Configs
    Loader& loader;
    std::vector<glm::mat4x4> model;
    glm::mat4x4 view;
    glm::mat4x4 projection;
    glm::mat4x4 screenspace;

    // Buffers
    ImageGrey ZBuffer;
    ImageGrey MSAA_mask;
    ImageBuffer<gBufferStruct> GBuffer;

    std::vector<Image> mipmap_vector;

    // Configurations
    /**
     * The default value for the ZBuffer during initialization.
     */
    static float zBufferDefault;
    static bool msaaMaskDefault;
    static gBufferStruct gBufferDefault;

    std::vector<glm::vec2> msaaSamples;
};

#endif
