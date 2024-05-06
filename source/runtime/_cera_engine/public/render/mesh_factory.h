#pragma once

#include "render/d3dx12_declarations.h"

#include <memory>

namespace cera
{
    class scene;
    class command_list;

    namespace mesh_factory
    {
        /**
         * Create a cube.
         *
         * @param size The size of one side of the cube.
         * @param reverseWinding Whether to reverse the winding order of the triangles (useful for skyboxes).
         */
        std::shared_ptr<scene> create_cube(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float size = 1.0);

        /**
         * Create a sphere.
         *
         * @param radius Radius of the sphere.
         * @param tessellation Determines how smooth the sphere is.
         * @param reverseWinding Whether to reverse the winding order of the triangles (useful for sydomes).
         */
        std::shared_ptr<scene> create_sphere(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float radius = 0.5f, uint32_t tessellation = 16);

        /**
         * Create a Cylinder
         *
         * @param radius The radius of the primary axis of the cylinder.
         * @param hight The height of the cylinder.
         * @param tessellation How smooth the cylinder will be.
         * @param reverseWinding Whether to reverse the winding order of the triangles.
         */
        std::shared_ptr<scene> create_cylinder(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float radius = 0.5f, float height = 1.0f, uint32_t tessellation = 32);

        /**
         * Create a plane.
         *
         * @param width The width of the plane.
         * @param height The height of the plane.
         * @reverseWinding Whether to reverse the winding order of the plane.
         */
        std::shared_ptr<scene> create_plane(const std::shared_ptr<command_list>& commandList, DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), float width = 1.0f, float height = 1.0f);
    }
}