#include "resources/resource.h"
#include "resources/resource_state_tracker.h"

#include "rhi_directx_call.h"
#include "rhi_directx_device.h"

#include "util/assert.h"
#include "util/log.h"

namespace cera
{
  namespace renderer
  {
    //-------------------------------------------------------------------------
    resource::resource(d3d12_device& device, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue)
        : m_device_ref(device)
    {
      auto d3d_device = m_device_ref.d3d_device();

      if(clearValue)
      {
        m_d3d_clear_value = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
      }

      auto default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

      auto hr = d3d_device->CreateCommittedResource(&default_heap_properties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, m_d3d_clear_value.get(), IID_PPV_ARGS(&m_d3d_resource));

      CERA_ASSERT_X(SUCCEEDED(hr), "Unable to CreateCommittedResource");

      ResourceStateTracker::add_global_resource_state(m_d3d_resource.Get(), D3D12_RESOURCE_STATE_COMMON);

      auto result = check_feature_support();

      CERA_ASSERT_X(result, "resource format is not supported");
    }

    //-------------------------------------------------------------------------
    resource::resource(d3d12_device& device, wrl::com_ptr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue)
        : m_device_ref(device)
        , m_d3d_resource(resource)
    {
      if(clearValue)
      {
        m_d3d_clear_value = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
      }

      auto result = check_feature_support();

      CERA_ASSERT_X(result, "resource format is not supported");
    }

    //-------------------------------------------------------------------------
    resource::~resource() = default;

    //-------------------------------------------------------------------------
    d3d12_device* resource::device() const
    {
      return &m_device_ref;
    }

    //-------------------------------------------------------------------------
    wrl::com_ptr<ID3D12Resource> resource::d3d_resource() const
    {
      return m_d3d_resource;
    }

    //-------------------------------------------------------------------------
    D3D12_RESOURCE_DESC resource::d3d_resource_desc() const
    {
      D3D12_RESOURCE_DESC res_desc = {};
      if(m_d3d_resource)
      {
        res_desc = m_d3d_resource->GetDesc();
      }

      return res_desc;
    }

    //-------------------------------------------------------------------------
    void resource::set_resource_name(const std::wstring& name)
    {
      m_resource_name = name;

      if(m_d3d_resource && !m_resource_name.empty())
      {
        m_d3d_resource->SetName(m_resource_name.c_str());
      }
    }

    //-------------------------------------------------------------------------
    bool resource::check_format_support(D3D12_FORMAT_SUPPORT1 formatSupport) const
    {
      return (m_format_support.Support1 & formatSupport) != 0;
    }

    //-------------------------------------------------------------------------
    bool resource::check_format_support(D3D12_FORMAT_SUPPORT2 formatSupport) const
    {
      return (m_format_support.Support2 & formatSupport) != 0;
    }

    //-------------------------------------------------------------------------
    const D3D12_CLEAR_VALUE* resource::d3d_clear_value() const
    {
      return m_d3d_clear_value.get();
    }

    //-------------------------------------------------------------------------
    const std::wstring& resource::resource_name() const
    {
      return m_resource_name;
    }

    //-------------------------------------------------------------------------
    bool resource::check_feature_support()
    {
      auto d3d_device = m_device_ref.d3d_device();

      auto desc = m_d3d_resource->GetDesc();

      m_format_support.Format = desc.Format;

      if(DX_FAILED(d3d_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &m_format_support, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT))))
      {
        log::error("Feature not supported on D3D12 resource");
        return false;
      }

      return true;
    }
  } // namespace renderer
} // namespace cera