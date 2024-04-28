#pragma once

#include "render/d3dx12_declarations.h"

namespace cera
{
    struct vertex_pos_color
    {
        explicit vertex_pos_color(const DirectX::XMFLOAT3& inPosition, const DirectX::XMFLOAT3& inColor)
            : position(inPosition)
            , color(inColor)
        {}

        explicit vertex_pos_color(DirectX::FXMVECTOR inPosition, DirectX::FXMVECTOR inColor)
        {
            DirectX::XMStoreFloat3(&(this->position), inPosition);
            DirectX::XMStoreFloat3(&(this->color), inColor);
        }

        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 color;

        static const D3D12_INPUT_LAYOUT_DESC    input_layout;
        static const int                        input_element_count = 2;
        static const D3D12_INPUT_ELEMENT_DESC   input_elements[input_element_count];
    };
}