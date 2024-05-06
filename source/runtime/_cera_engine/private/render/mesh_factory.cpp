#include "render/mesh_factory.h"
#include "render/device.h"
#include "render/vertex_buffer.h"
#include "render/index_buffer.h"
#include "render/vertex_types.h"
#include "render/command_list.h"
#include "mesh.h"
#include "scene_node.h"
#include "scene.h"

namespace cera
{
    namespace mesh_factory
    {
        using vertex_collection = std::vector<vertex_pos_color>;
        using index_collection = std::vector<uint16_t>;

        namespace internal
        {
            std::shared_ptr<scene> create_scene(const std::shared_ptr<command_list>& commandList, const vertex_collection& vertices, const index_collection& indices)
            {
                if (vertices.empty())
                {
                    return nullptr;
                }

                auto new_vertex_buffer = commandList->copy_vertex_buffer(vertices);
                auto new_index_Buffer = commandList->copy_index_buffer(indices);

                auto new_mesh = std::make_shared<mesh>();

                new_mesh->set_vertex_buffer(0, new_vertex_buffer);
                new_mesh->set_index_buffer(new_index_Buffer);

                auto new_node = std::make_shared<scene_node>();
                new_node->add_mesh(new_mesh);

                auto new_scene = std::make_shared<scene>();
                new_scene->set_root_node(new_node);

                return new_scene;
            }

            // Definition for inline functions.
            inline DirectX::XMVECTOR get_circle_vector(size_t i, size_t tessellation) noexcept
            {
                float angle = float(i) * DirectX::XM_2PI / float(tessellation);
                float dx, dz;

                DirectX::XMScalarSinCos(&dx, &dz, angle);

                DirectX::XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
                return v;
            }

            void create_cylinder_cap(vertex_collection& vertices, index_collection& indices, size_t tessellation, float height, float radius, bool isTop, DirectX::XMFLOAT3 color)
            {
                // Create cap indices.
                for (size_t i = 0; i < tessellation - 2; i++)
                {
                    size_t i1 = (i + 1) % tessellation;
                    size_t i2 = (i + 2) % tessellation;

                    if (isTop)
                    {
                        std::swap(i1, i2);
                    }

                    size_t vbase = vertices.size();
                    indices.push_back(vbase + i2);
                    indices.push_back(vbase + i1);
                    indices.push_back(vbase);
                }

                // Which end of the cylinder is this?
                DirectX::XMVECTOR normal = DirectX::g_XMIdentityR1;

                if (!isTop)
                {
                    normal = DirectX::XMVectorNegate(normal);
                }

                // Create cap vertices.
                for (size_t i = 0; i < tessellation; i++)
                {
                    DirectX::XMVECTOR circle_vector = get_circle_vector(i, tessellation);
                    DirectX::XMVECTOR position = DirectX::XMVectorAdd(DirectX::XMVectorScale(circle_vector, radius), DirectX::XMVectorScale(normal, height));
                    vertices.emplace_back(position, DirectX::XMLoadFloat3(&color));
                }
            }
        }

        /**
         * Create a cube.
         *
         * @param size The size of one side of the cube.
         * @param reverseWinding Whether to reverse the winding order of the triangles (useful for skyboxes).
         */
        std::shared_ptr<scene> create_cube(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color, float size)
        {
            // Cube is centered at 0,0,0.
            float s = size * 0.5f;

            // 8 vertex position
            DirectX::XMFLOAT3 p[8] = 
            { 
                { +s, +s, -s }, 
                { +s, +s, +s },
                { +s, -s, +s },
                { +s, -s, -s },
                { -s, +s, +s },
                { -s, +s, -s }, 
                { -s, -s, -s }, 
                { -s, -s, +s }
            };

            // Indices for the vertex positions.
            uint16_t i[24] = {
                0, 1, 2, 3,  // +X
                4, 5, 6, 7,  // -X
                4, 1, 0, 5,  // +Y
                2, 7, 6, 3,  // -Y
                1, 4, 7, 2,  // +Z
                5, 0, 3, 6   // -Z
            };

            vertex_collection vertices;
            index_collection indices;

            for (uint16_t f = 0; f < 6; ++f)  // For each face of the cube.
            {
                // Four vertices per face.
                vertices.emplace_back(p[i[f * 4 + 0]], color);
                vertices.emplace_back(p[i[f * 4 + 1]], color);
                vertices.emplace_back(p[i[f * 4 + 2]], color);
                vertices.emplace_back(p[i[f * 4 + 3]], color);

                // First triangle.
                indices.emplace_back(f * 4 + 0);
                indices.emplace_back(f * 4 + 1);
                indices.emplace_back(f * 4 + 2);

                // Second triangle
                indices.emplace_back(f * 4 + 2);
                indices.emplace_back(f * 4 + 3);
                indices.emplace_back(f * 4 + 0);
            }

            return internal::create_scene(commandList, vertices, indices);
        }

        /**
         * Create a sphere.
         *
         * @param radius Radius of the sphere.
         * @param tessellation Determines how smooth the sphere is.
         * @param reverseWinding Whether to reverse the winding order of the triangles (useful for sydomes).
         */
        std::shared_ptr<scene> create_sphere(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color, float radius, uint32_t tessellation)
        {
            assert(tessellation > 3 && "tessellation parameter out of range");

            vertex_collection vertices;
            index_collection  indices;

            size_t vertical_segments = tessellation;
            size_t horizontal_segments = tessellation * 2;

            // Create rings of vertices at progressively higher latitudes.
            for (size_t i = 0; i <= vertical_segments; i++)
            {
                float v = 1 - (float)i / vertical_segments;

                float latitude = (i * DirectX::XM_PI / vertical_segments) - DirectX::XM_PIDIV2;
                float dy, dxz;

                DirectX::XMScalarSinCos(&dy, &dxz, latitude);

                // Create a single ring of vertices at this latitude.
                for (size_t j = 0; j <= horizontal_segments; j++)
                {
                    float u = (float)j / horizontal_segments;

                    float longitude = j * DirectX::XM_2PI / horizontal_segments;
                    float dx, dz;

                    DirectX::XMScalarSinCos(&dx, &dz, longitude);

                    dx *= dxz;
                    dz *= dxz;

                    auto normal = DirectX::XMVectorSet(dx, dy, dz, 0);
                    auto position = DirectX::operator*(radius, normal);

                    vertices.emplace_back(position, DirectX::XMLoadFloat3(&color));
                }
            }

            // Fill the index buffer with triangles joining each pair of latitude rings.
            size_t stride = horizontal_segments + 1;

            for (size_t i = 0; i < vertical_segments; i++)
            {
                for (size_t j = 0; j <= horizontal_segments; j++)
                {
                    size_t nextI = i + 1;
                    size_t nextJ = (j + 1) % stride;

                    indices.push_back(i * stride + nextJ);
                    indices.push_back(nextI * stride + j);
                    indices.push_back(i * stride + j);

                    indices.push_back(nextI * stride + nextJ);
                    indices.push_back(nextI * stride + j);
                    indices.push_back(i * stride + nextJ);
                }
            }

            return internal::create_scene(commandList, vertices, indices);
        }

        /**
         * Create a Cylinder
         *
         * @param radius The radius of the primary axis of the cylinder.
         * @param hight The height of the cylinder.
         * @param tessellation How smooth the cylinder will be.
         * @param reverseWinding Whether to reverse the winding order of the triangles.
         */
        std::shared_ptr<scene> create_cylinder(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color, float radius, float height, uint32_t tessellation)
        {
            assert(tessellation > 3 && "tessellation parameter out of range");

            vertex_collection vertices;
            index_collection  indices;

            height /= 2;

            DirectX::XMVECTOR top_offset = DirectX::XMVectorScale(DirectX::g_XMIdentityR1, height);

            size_t stride = tessellation + 1;

            DirectX::XMVECTOR simd_color = DirectX::XMLoadFloat3(&color);

            // Create a ring of triangles around the outside of the cylinder.
            for (size_t i = 0; i <= tessellation; i++)
            {
                DirectX::XMVECTOR normal = internal::get_circle_vector(i, tessellation);
                DirectX::XMVECTOR side_offset = DirectX::XMVectorScale(normal, radius);

                float u = float(i) / float(tessellation);

                vertices.emplace_back(DirectX::XMVectorAdd(side_offset, top_offset), simd_color);
                vertices.emplace_back(DirectX::XMVectorSubtract(side_offset, top_offset), simd_color);

                indices.push_back(i * 2 + 1);
                indices.push_back((i * 2 + 2) % (stride * 2));
                indices.push_back(i * 2);

                indices.push_back((i * 2 + 3) % (stride * 2));
                indices.push_back((i * 2 + 2) % (stride * 2));
                indices.push_back(i * 2 + 1);
            }

            // Create flat triangle fan caps to seal the top and bottom.
            internal::create_cylinder_cap(vertices, indices, tessellation, height, radius, true, color);
            internal::create_cylinder_cap(vertices, indices, tessellation, height, radius, false, color);

            return internal::create_scene(commandList, vertices, indices);
        }

        /**
         * Create a plane.
         *
         * @param width The width of the plane.
         * @param height The height of the plane.
         * @reverseWinding Whether to reverse the winding order of the plane.
         */
        std::shared_ptr<scene> create_plane(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color, float width, float height)
        {
            // clang-format off
            // Define a plane that is aligned with the X-Z plane and the normal is facing up in the Y-axis.
            vertex_collection vertices = 
            {
                vertex_pos_color(DirectX::XMFLOAT3(-0.5f * width, 0.0f, 0.5f * height), color),  // 0
                vertex_pos_color(DirectX::XMFLOAT3(0.5f * width, 0.0f, 0.5f * height), color),   // 1
                vertex_pos_color(DirectX::XMFLOAT3(0.5f * width, 0.0f, -0.5f * height), color),  // 2
                vertex_pos_color(DirectX::XMFLOAT3(-0.5f * width, 0.0f, -0.5f * height), color)  // 3
            };
            
            index_collection indices = { 1, 3, 0, 2, 3, 1 };

            return internal::create_scene(commandList, vertices, indices);
        }
    }
}