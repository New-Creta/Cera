#include "render/d3dx12_call.h"

#include "util/log.h"

#include <comdef.h>

namespace cera
{
    namespace render
    {
        namespace directx
        {
            //-------------------------------------------------------------------------
            std::string report_hr_error(HRESULT hr, const std::string_view file, const std::string_view function, s32 lineNr)
            {
                const _com_error err(hr);
                std::string error_message(err.ErrorMessage());
                log::error("DirectX Error");
                log::error("File: {}", file);
                log::error("Function: {}", function);
                log::error("On line: {}", lineNr);
                log::error("DXdows error: {}", error_message);

                return error_message;
            }

            //-------------------------------------------------------------------------
            DXCall::DXCall(HResult error, std::string_view file, std::string_view function, s32 lineNr)
                : m_error(error)
            {
                if (FAILED(m_error))
                {
                    m_error_message = report_hr_error(m_error, file, function, lineNr);
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
                return m_error_message;
            }
        } // namespace directx
    }   // namespace renderer
} // namespace cera