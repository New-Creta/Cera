#include "rhi_directx_call.h"
#include "rhi_directx_util.h"

#include "util/log.h"
#include "util/string_op.h"

#include <comdef.h>

namespace cera
{
  namespace renderer
  {
    namespace directx
    {
      //-------------------------------------------------------------------------
      std::string report_hr_error(HRESULT hr, const std::string_view file, const std::string_view function, s32 line_nr)
      {
        const _com_error err(hr);
        
        std::wstring unicode_error_message(err.ErrorMessage());
        std::string multibyte_error_message = string_operations::to_multibyte(unicode_error_message.c_str(), unicode_error_message.size());
        
        log::error("DirectX Error\nFile: {}\nFunction: {}\nOn line: {}\nDX error: {}", file, function, line_nr, multibyte_error_message.c_str());
        
        return multibyte_error_message;
      }

      //-------------------------------------------------------------------------
      DXCall::DXCall(HResult error, std::string_view file, std::string_view function, s32 line_nr) : m_error(error)
      {
        if(FAILED(m_error))
        {
          m_error_message  = report_hr_error(m_error, file, function, line_nr);
        }
      }

      //-------------------------------------------------------------------------
      bool DXCall::has_failed() const
      {
        return FAILED(m_error);
      }

      //-------------------------------------------------------------------------
      bool DXCall::has_succeeded() const
      {
        return !has_failed();
      }

      //-------------------------------------------------------------------------
      const std::string& DXCall::error_message() const
      {
        return m_error_message;
      }
    } // namespace directx
  }   // namespace renderer
} // namespace cera