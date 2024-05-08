#include "directx_call.h"

#include "directx_util.h"
#include "log.h"

#include <comdef.h>

namespace cera
{
  namespace renderer
  {
    namespace directx
    {
      //-------------------------------------------------------------------------
      std::big_stack_string report_hr_error(HRESULT hr, const std::string_view file, const std::string_view function, s32 lineNr)
      {
        const _com_error err(hr);
        std::big_stack_string error_message(err.ErrorMessage());
        CERA_ERROR(LogDirectX, "DirectX Error\nFile: {}\nFunction: {}\nOn line: {}\nDX error: {}", file, function, lineNr, error_message);
        return error_message;
      }

      //-------------------------------------------------------------------------
      DXCall::DXCall(HResult error, std::string_view file, std::string_view function, s32 lineNr)
          : m_error(error)
      {
        if(FAILED(m_error))
        {
          m_error_message  = report_hr_error(m_error, file, function, lineNr);
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
      std::string_view DXCall::error_message() const
      {
        return m_error_message.to_view();
      }
    } // namespace directx
  }   // namespace renderer
} // namespace cera