#include "directx_call.h"
#include "directx_util.h"

#include "util/log.h"

#include <comdef.h>

namespace cera
{
  namespace renderer
  {
    namespace directx
    {
      //-------------------------------------------------------------------------
      std::wstring report_hr_error(HRESULT hr, const std::wstring_view file, const std::wstring_view function,
                                            s32 line_nr)
      {
        const _com_error err(hr);
        std::wstring error_message(err.ErrorMessage());
        log::error(L"DirectX Error\nFile: {}\nFunction: {}\nOn line: {}\nDX error: {}", file, function, line_nr, error_message.c_str());
        return error_message;
      }

      //-------------------------------------------------------------------------
      DXCall::DXCall(HResult error, std::wstring_view file, std::wstring_view function, s32 line_nr) : m_error(error)
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
      const std::wstring& DXCall::error_message() const
      {
        return m_error_message;
      }
    } // namespace directx
  }   // namespace renderer
} // namespace cera