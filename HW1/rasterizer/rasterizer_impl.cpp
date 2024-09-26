#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include <_types/_uint32_t.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb/stb_image.h"
#include "entities.hpp"
#include "image.hpp"
#include "loader.hpp"
#include "rasterizer.hpp"

// TODO
const glm::vec3 pixel_center = glm::vec3(0.5F, 0.5F, 0.0F);

void printMat4(glm::mat4 mat) {
    mat = glm::transpose(mat);
    std::cout << mat[0][0] << "\t" << mat[0][1] << "\t" << mat[0][2] << "\t" << mat[0][3] << std::endl;
    std::cout << mat[1][0] << "\t" << mat[1][1] << "\t" << mat[1][2] << "\t" << mat[1][3] << std::endl;
    std::cout << mat[2][0] << "\t" << mat[2][1] << "\t" << mat[2][2] << "\t" << mat[2][3] << std::endl;
    std::cout << mat[3][0] << "\t" << mat[3][1] << "\t" << mat[3][2] << "\t" << mat[3][3] << std::endl;
}

bool sampleIsInsideTriangle(glm::vec3 sample_pos, Triangle trig) {}

void Rasterizer::DrawPixel(uint32_t x, uint32_t y, Triangle trig, AntiAliasConfig config, uint32_t spp, Image& image,
                           Color color) {}

// TODO
void Rasterizer::AddModel(MeshTransform transform, glm::mat4 rotation) {}

// TODO
void Rasterizer::SetView() {}

// TODO
void Rasterizer::SetProjection() {}

// TODO
void Rasterizer::SetScreenSpace() {}

// TODO
glm::vec3 Rasterizer::BarycentricCoordinate(glm::vec2 pos, Triangle trig) {}

float Rasterizer::zBufferDefault = -1.0F;
void Rasterizer::UpdateDepthAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                                    ImageGrey& ZBuffer) {}

bool Rasterizer::msaaMaskDefault = false;
void Rasterizer::UpdateMSAAAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                                   ImageGrey& MSAAMask) {}

Rasterizer::gBufferStruct Rasterizer::gBufferDefault { glm::vec3(), glm::vec3() };
void Rasterizer::UpdateGBufferAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                                      ImageBuffer<gBufferStruct>& gBuffer) {}

void readImageIn(const std::string& filename, Image& target, const std::string& output_file) {
    int width, height, nrChannels;
    auto img = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    target = Image(width, height, output_file);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            int index = (x + y * width) * nrChannels;
            if (nrChannels == 4) {
                target.Set(x, y, Color(img[index], img[index + 1], img[index + 2], img[index + 3]));
            } else if (nrChannels == 3) {
                glm::vec3 color_vec(img[index], img[index + 1], img[index + 2]);
                target.Set(x, y, Color(color_vec));
            }
        }
    }
    stbi_image_free(img);
}

void Rasterizer::CreateMipMap(const std::string& texture_filename) {
    Image original_texture;
    std::string mipmap_file = std::string("texture-mipmap/level-0");
    readImageIn(texture_filename, original_texture, mipmap_file);

    uint32_t width = original_texture.GetWidth();
    uint32_t height = original_texture.GetWidth();
}

Color Rasterizer::GetTexel(glm::vec2 tex_coord, float depth) {}

void Rasterizer::ShadeAtPixel(uint32_t x, uint32_t y, Image& image) {}

void Rasterizer::ShadeAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, Image& image) {}
