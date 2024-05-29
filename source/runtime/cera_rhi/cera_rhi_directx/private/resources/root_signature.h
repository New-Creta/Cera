#pragma once

#include "util/types.h"
#include "util/windows_types.h"

#include "rhi_directx_util.h"

#include "resources/rhi_resource.h"

namespace cera
{
    namespace renderer
    {
        class d3d12_device;

        class RootSignature : public rhi_resource
        {
        public:
            RESOURCE_CLASS_TYPE(RootSignature);

            wrl::com_ptr<ID3D12RootSignature> d3d_root_signature() const;
            D3D12_ROOT_SIGNATURE_DESC1 d3d_root_signature_description() const;

            u32 descriptor_table_bit_mask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
            u32 num_descriptors(u32 rootIndex) const;

        protected:
            RootSignature(d3d12_device& device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);

            virtual ~RootSignature();

        private:
            bool set_root_signature_desc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc);
            void destroy();

        private:
            d3d12_device& m_device;

            D3D12_ROOT_SIGNATURE_DESC1 m_root_signature_description;

            wrl::com_ptr<ID3D12RootSignature> m_root_signature;

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
}