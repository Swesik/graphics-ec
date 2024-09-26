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

bool sampleIsInsideTriangle(glm::vec3 sample_pos, Triangle trig) {
    glm::vec3 a1(trig.pos[0].x, trig.pos[0].y, 0.0F);
    glm::vec3 a2(trig.pos[1].x, trig.pos[1].y, 0.0F);
    glm::vec3 a3(trig.pos[2].x, trig.pos[2].y, 0.0F);
    glm::vec3 s1 = glm::cross(sample_pos - a1, a2 - a1);
    glm::vec3 s2 = glm::cross(sample_pos - a2, a3 - a2);
    glm::vec3 s3 = glm::cross(sample_pos - a3, a1 - a3);
    glm::vec3 screen_normal = glm::vec3(0, 0, 1);

    return (glm::dot(s1, screen_normal) > 0.0F && glm::dot(s2, screen_normal) > 0.0F
            && glm::dot(s3, screen_normal) > 0.0F)
        || (glm::dot(s1, screen_normal) < 0.0F && glm::dot(s2, screen_normal) < 0.0F
            && glm::dot(s3, screen_normal) < 0.0F);
}

void Rasterizer::DrawPixel(uint32_t x, uint32_t y, Triangle trig, AntiAliasConfig config, uint32_t spp, Image& image,
                           Color color) {
    glm::vec3 pixel(x, y, 0.0F);
    if (config == AntiAliasConfig::NONE)   // if anti-aliasing is off
    {
        glm::vec3 sample_pos = pixel + pixel_center;
        if (sampleIsInsideTriangle(sample_pos, trig)) {
            image.Set(x, y, color);
            return;
        }

    } else if (config == AntiAliasConfig::SSAA) {   // if anti-aliasing is on
        uint32_t num_inside = 0;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0F, 1.0);
        for (uint32_t i = 0; i < spp; i++) {
            glm::vec3 x = pixel + glm::vec3(dis(gen), dis(gen), 0);

            if (sampleIsInsideTriangle(x, trig)) {
                num_inside++;
            }
        }
        color = (static_cast<float>(num_inside) / static_cast<float>(spp)) * color;
        image.Set(x, y, color);
        return;
    }
}

// TODO
void Rasterizer::AddModel(MeshTransform transform, glm::mat4 rotation) {
    glm::mat4 scale(transform.scale.x, 0.0F, 0.0F, 0.0F,   // first column
                    0.0F, transform.scale.y, 0.0F, 0.0F,   // second column
                    0.0F, 0.0F, transform.scale.z, 0.0F,   // third column
                    0.0F, 0.0F, 0.0F, 1.0F                 // fourth column
    );

    glm::mat4 translation(1.0F, 0.0F, 0.0F, 0.0F,   // first column
                          0.0F, 1.0F, 0.0F, 0.0F,   // second column
                          0.0F, 0.0F, 1.0F, 0.0F,   // third colum
                          transform.translation.x, transform.translation.y, transform.translation.z,
                          1.0F   // fourth column
    );

    model.push_back(translation * rotation * scale);
    // printMat4(model.back());
}

// TODO
void Rasterizer::SetView() {
    const Camera& camera = this->loader.GetCamera();
    glm::vec3 camera_pos = camera.pos;
    glm::vec3 camera_look_at = camera.lookAt;

    glm::vec3 gaze = glm::normalize(camera_look_at - camera_pos);
    glm::vec3 camera_side = glm::normalize(glm::cross(gaze, camera.up));
    glm::vec3 up = glm::normalize(camera.up);

    // TODO change this line to the correct view matrix
    glm::mat4 translate_to_origin(1.0F, 0.0F, 0.0F, 0.0F,                             // first column
                                  0.0F, 1.0F, 0.0F, 0.0F,                             // second column
                                  0.0F, 0.0F, 1.0F, 0.0F,                             // third colum
                                  -camera_pos.x, -camera_pos.y, -camera_pos.z, 1.0F   // fourth column
    );

    glm::mat4 model_to_view(camera_side.x, up.x, -gaze.x, 0.0F,   // first column
                            camera_side.y, up.y, -gaze.y, 0.0F,   // second column
                            camera_side.z, up.z, -gaze.z, 0.0F,   // third column
                            0.0F, 0.0F, 0.0F, 1.0F                // fourth column
    );
    this->view = model_to_view * translate_to_origin;
}

// TODO
void Rasterizer::SetProjection() {
    const Camera& camera = this->loader.GetCamera();

    float near_clip = -camera.nearClip;   // near clipping distance, strictly positive
    float far_clip = -camera.farClip;     // far clipping distance, strictly positive

    float width = static_cast<float>(camera.width);
    float height = static_cast<float>(camera.height);

    // TODO change this line to the correct projection matrix
    glm::mat4 tranlsate_to_center(1.0F, 0.0F, 0.0F, 0.0F,                         // first column
                                  0.0F, 1.0F, 0.0F, 0.0F,                         // second column
                                  0.0F, 0.0F, 1.0F, 0.0F,                         // third colum
                                  0.0F, 0.0F, -(near_clip + far_clip) / 2, 1.0F   // fourth column
    );

    glm::mat4 scale_to_bounding_box(2.0F / width, 0.0F, 0.0F, 0.0F,                 // first column
                                    0.0F, 2 / height, 0.0F, 0.0F,                   // second column
                                    0.0F, 0.0F, 2 / (near_clip - far_clip), 0.0F,   // third column
                                    0.0F, 0.0F, 0.0F, 1.0F                          // fourth column
    );

    glm::mat4 perspective(near_clip, 0.0F, 0.0F, 0.0F,              // first column,
                          0.0F, near_clip, 0.0F, 0.0F,              // second column,
                          0.0F, 0.0F, near_clip + far_clip, 1.0F,   // third column,
                          0.0F, 0.0F, -near_clip * far_clip, 0.0F   // fourth column,
    );

    this->projection = scale_to_bounding_box * tranlsate_to_center * perspective;
    // printMat4(this->projection);
}

// TODO
void Rasterizer::SetScreenSpace() {
    float width = static_cast<float>(this->loader.GetWidth());
    float height = static_cast<float>(this->loader.GetHeight());

    // TODO change this line to the correct screenspace matrix
    this->screenspace = glm::mat4(width / 2, 0.0F, 0.0F, 0.0F,        // first column
                                  0.0F, height / 2, 0.0F, 0.0F,       // second column
                                  0.0F, 0.0F, 1.0F, 0.0F,             // third column
                                  width / 2, height / 2, 0.0F, 1.0F   // fourth column
    );
}

// TODO
glm::vec3 Rasterizer::BarycentricCoordinate(glm::vec2 pos, Triangle trig) {
    float alpha = (-(pos.x - trig.pos[1].x) * (trig.pos[2].y - trig.pos[1].y)
                   + (pos.y - trig.pos[1].y) * (trig.pos[2].x - trig.pos[1].x))
                / (-(trig.pos[0].x - trig.pos[1].x) * (trig.pos[2].y - trig.pos[1].y)
                   + (trig.pos[0].y - trig.pos[1].y) * (trig.pos[2].x - trig.pos[1].x));
    float beta = (-(pos.x - trig.pos[2].x) * (trig.pos[0].y - trig.pos[2].y)
                  + (pos.y - trig.pos[2].y) * (trig.pos[0].x - trig.pos[2].x))
               / (-(trig.pos[1].x - trig.pos[2].x) * (trig.pos[0].y - trig.pos[2].y)
                  + (trig.pos[1].y - trig.pos[2].y) * (trig.pos[0].x - trig.pos[2].x));
    float gamma = 1 - alpha - beta;
    return glm::vec3(alpha, beta, gamma);
}

float Rasterizer::zBufferDefault = -1.0F;
void Rasterizer::UpdateDepthAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                                    ImageGrey& ZBuffer) {
    glm::vec2 pos(static_cast<float>(x) + 0.5F, static_cast<float>(y) + 0.5F);
    glm::vec3 bary_coord = BarycentricCoordinate(pos, transformed);

    if (bary_coord.x < 0 || bary_coord.y < 0 || bary_coord.z < 0) {
        return;
    }
    float inv_result = bary_coord.x * (1 / transformed.pos[0].z) + bary_coord.y * (1 / transformed.pos[1].z)
                     + bary_coord.z * (1 / transformed.pos[2].z);
    if (1 / inv_result > ZBuffer.Get(x, y)) {
        ZBuffer.Set(x, y, 1 / inv_result);
    }
}

bool Rasterizer::msaaMaskDefault = false;
void Rasterizer::UpdateMSAAAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                                   ImageGrey& MSAAMask) {
    glm::vec3 bary_coord
      = BarycentricCoordinate(glm::vec2(static_cast<float>(x) + 0.5F, static_cast<float>(y) + 0.5F), transformed);
    float inv_depth = bary_coord.x * (1 / transformed.pos[0].z) + bary_coord.y * (1 / transformed.pos[1].z)
                    + bary_coord.z * (1 / transformed.pos[2].z);
    if (ZBuffer.Get(x, y) != 1 / inv_depth) {
        return;
    }

    glm::vec3 pos(x, y, 0);
    uint32_t num_contained_samples = 0;
    for (const auto& sample : msaaSamples) {
        num_contained_samples += sampleIsInsideTriangle(pos + glm::vec3(sample, 0.0F), transformed) ? 1 : 0;
    }
    MSAAMask.Set(x, y, static_cast<float>(num_contained_samples) / static_cast<float>(msaaSamples.size()));
}

Rasterizer::gBufferStruct Rasterizer::gBufferDefault { glm::vec3(), glm::vec3() };
void Rasterizer::UpdateGBufferAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed,
                                      ImageBuffer<gBufferStruct>& gBuffer) {
    if (loader.GetAntiAliasConfig() == AntiAliasConfig::MSAA && MSAA_mask.Get(x, y) == 0.0F) {
        return;
    }

    glm::vec2 pos(static_cast<float>(x) + 0.5F, static_cast<float>(y) + 0.5F);
    glm::vec3 bary_coord = BarycentricCoordinate(pos, transformed);

    float inv_depth = bary_coord.x * (1 / transformed.pos[0].z) + bary_coord.y * (1 / transformed.pos[1].z)
                    + bary_coord.z * (1 / transformed.pos[2].z);
    if (ZBuffer.Get(x, y) != 1 / inv_depth) {
        return;
    }

    glm::vec3 norm = glm::normalize(bary_coord.x * original.normal[0] + bary_coord.y * original.normal[1]
                                    + bary_coord.z * original.normal[2]);
    glm::vec3 world_coord
      = bary_coord.x * original.pos[0] + bary_coord.y * original.pos[1] + bary_coord.z * original.pos[2];

    if (loader.GetTextureName().empty()) {
        gBuffer.Set(x, y, { norm, world_coord, Color() });
        return;
    }

    glm::vec2 tex_coord = bary_coord.x * transformed.tex_coord[0] + bary_coord.y * transformed.tex_coord[1]
                        + bary_coord.z * transformed.tex_coord[2];
    Color texel = GetTexel(tex_coord, 1 / inv_depth);
    gBuffer.Set(x, y, { norm, world_coord, texel });
}

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
    uint32_t num_mipmaps = static_cast<uint32_t>(std::log2(std::min(width, height)));
    this->mipmap_vector.reserve(num_mipmaps);
    this->mipmap_vector.emplace_back(original_texture);

    if (loader.GetType() == TestType::TEXTURE_TEST) {
        this->mipmap_vector[0].Write();
    }
    // num_mipmaps = 1;
    for (uint32_t i = 1; i <= num_mipmaps; ++i) {
        mipmap_file = "texture-mipmap/level-" + std::to_string(i);
        mipmap_vector.emplace_back(mipmap_vector[i - 1].GetWidth() / 2, mipmap_vector[i - 1].GetHeight() / 2,
                                   mipmap_file);
        for (uint32_t x = 0; x < mipmap_vector[i].GetWidth(); ++x) {
            for (uint32_t y = 0; y < mipmap_vector[i].GetHeight(); ++y) {
                float num_samples = 4;
                Color color1 = (1 / num_samples) * mipmap_vector[i - 1].Get(2 * x, 2 * y).value_or(Color());
                Color color2 = (1 / num_samples) * mipmap_vector[i - 1].Get(2 * x + 1, 2 * y).value_or(Color());
                Color color3 = (1 / num_samples) * mipmap_vector[i - 1].Get(2 * x, 2 * y + 1).value_or(Color());
                Color color4 = (1 / num_samples) * mipmap_vector[i - 1].Get(2 * x + 1, 2 * y + 1).value_or(Color());
                mipmap_vector[i].Set(x, y, (color1 + color2 + color3 + color4));
            }
        }
        // (1,0) --> (2,0),(2,1),(3,0),(3,1)
        if (loader.GetType() == TestType::TEXTURE_TEST) {
            mipmap_vector[i].Write();
        }
    }
}

// TODO
Color Rasterizer::GetTexel(glm::vec2 tex_coord, float depth) {
    uint32_t mip_map_level = static_cast<uint32_t>(static_cast<float>(mipmap_vector.size()) * (1.0F + depth) / 2);
    Image& mipmap = mipmap_vector[mip_map_level];
    Color texel = mipmap
                    .Get(static_cast<uint32_t>(mipmap.GetWidth() * tex_coord.x),
                         static_cast<uint32_t>(mipmap.GetHeight() * tex_coord.y))
                    .value();
    return texel;
}

void Rasterizer::ShadeAtPixel(uint32_t x, uint32_t y, Image& image) {
    if (!GBuffer.Get(x, y)) {
        return;
    }

    auto gBuffer_obj = *GBuffer.Get(x, y);
    Color result = loader.GetAmbientColor();

    if (!loader.GetTextureName().empty()) {
        Color texel = gBuffer_obj.texel;
        result = Color(static_cast<float>(result.r) * static_cast<float>(texel.r),
                       static_cast<float>(result.g) * static_cast<float>(texel.g),
                       static_cast<float>(result.b) * static_cast<float>(texel.b), static_cast<float>(texel.a));
    }

    const std::vector<Light>& lights = loader.GetLights();
    const Camera& camera = loader.GetCamera();

    for (auto& light : lights) {
        glm::vec3 light_ray = light.pos - gBuffer_obj.pos;
        glm::vec3 reflection_ray = camera.pos - gBuffer_obj.pos;
        glm::vec3 half_vec = glm::normalize(light_ray + reflection_ray);

        float r = glm::length(light_ray);
        Color diffuse
          = light.intensity * (1 / (r * r)) * std::max(0.0F, glm::dot(light_ray, gBuffer_obj.norm)) * light.color;
        Color specular = light.intensity * (1 / (r * r))
                       * std::pow(std::max(0.0F, glm::dot(half_vec, gBuffer_obj.norm)), loader.GetSpecularExponent())
                       * light.color;
        result = result + diffuse + specular;
    }

    if (loader.GetAntiAliasConfig() == AntiAliasConfig::MSAA) {
        image.Set(x, y, *MSAA_mask.Get(x, y) * result);
    }

    image.Set(x, y, result);
}

void Rasterizer::ShadeAtPixel(uint32_t x, uint32_t y, Triangle original, Triangle transformed, Image& image) {
    UpdateGBufferAtPixel(x, y, original, transformed, this->GBuffer);
    if (loader.GetAntiAliasConfig() == AntiAliasConfig::MSAA && MSAA_mask.Get(x, y) == 0.0F) {
        return;
    }

    glm::vec2 pos(static_cast<float>(x) + 0.5F, static_cast<float>(y) + 0.5F);
    glm::vec3 bary_coord = BarycentricCoordinate(pos, transformed);

    float inv_depth = bary_coord.x * (1 / transformed.pos[0].z) + bary_coord.y * (1 / transformed.pos[1].z)
                    + bary_coord.z * (1 / transformed.pos[2].z);
    if (ZBuffer.Get(x, y) != 1 / inv_depth) {
        return;
    }

    glm::vec3 norm = glm::normalize(bary_coord.x * original.normal[0] + bary_coord.y * original.normal[1]
                                    + bary_coord.z * original.normal[2]);
    glm::vec3 world_coord
      = bary_coord.x * original.pos[0] + bary_coord.y * original.pos[1] + bary_coord.z * original.pos[2];

    Color result = loader.GetAmbientColor();

    if (!loader.GetTextureName().empty()) {
        glm::vec2 tex_coord = bary_coord.x * transformed.tex_coord[0] + bary_coord.y * transformed.tex_coord[1]
                            + bary_coord.z * transformed.tex_coord[2];
        Color texel = GetTexel(tex_coord, 1 / inv_depth);
        result = Color(static_cast<float>(result.r) * static_cast<float>(texel.r),
                       static_cast<float>(result.g) * static_cast<float>(texel.g),
                       static_cast<float>(result.b) * static_cast<float>(texel.b), static_cast<float>(texel.a));
    }

    const std::vector<Light>& lights = loader.GetLights();
    const Camera& camera = loader.GetCamera();

    for (auto& light : lights) {
        glm::vec3 light_ray = light.pos - world_coord;
        glm::vec3 reflection_ray = camera.pos - world_coord;
        glm::vec3 half_vec = glm::normalize(light_ray + reflection_ray);

        float r = glm::length(light_ray);
        Color diffuse = light.intensity * (1 / (r * r)) * std::max(0.0F, glm::dot(light_ray, norm)) * light.color;
        Color specular = light.intensity * (1 / (r * r))
                       * std::pow(std::max(0.0F, glm::dot(half_vec, norm)), loader.GetSpecularExponent()) * light.color;
        result = result + diffuse + specular;

        // result = result + diffuse;
    }

    if (loader.GetAntiAliasConfig() == AntiAliasConfig::MSAA) {
        image.Set(x, y, *MSAA_mask.Get(x, y) * result);
    }

    image.Set(x, y, result);
}
