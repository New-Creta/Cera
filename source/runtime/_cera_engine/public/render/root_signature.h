#pragma once

#include "util/types.h"

#include "device/windows_types.h"

#include "render/d3dx12_declarations.h"

namespace cera
{
    class device;

    class root_signature
    {
    public:
        wrl::ComPtr<ID3D12RootSignature> get_d3d_root_signature() const;
        D3D12_ROOT_SIGNATURE_DESC1 get_d3d_root_signature_description() const;

        u32 get_descriptor_table_bit_mask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
        u32 get_num_descriptors(u32 rootIndex) const;

    protected:
        root_signature(device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

        virtual ~root_signature();

    private:
        bool set_root_signature_desc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);
        void destroy();

    private:
        device& m_device;

        D3D12_ROOT_SIGNATURE_DESC1 m_root_signature_description;

        wrl::ComPtr<ID3D12RootSignature> m_root_signature;

        // Need to know the number of descriptors per descriptor table.
        // A maximum of 32 descriptor tables are supported (since a 32-bit
        // mask is used to represent the descriptor tables in the root signature.
        u32 m_num_descriptors_per_table[32];

        // A bit mask that represents the root parameter indices that are 
        // descriptor tables for Samplers.
        u32 m_sampler_table_bit_mask;
        // A bit mask that represents the root parameter indices that are 
        // CBV, UAV, and SRV descriptor tables.
        u32 m_descriptor_table_bit_mask;
    };
}