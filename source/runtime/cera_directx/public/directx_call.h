#pragma once

#include "util/types.h"

#include <string>

namespace cera
{
  namespace renderer
  {
    namespace directx
    {
      using HResult = long;

      class DXCall
      {
      public:
        DXCall(HResult error, std::string_view file, std::string_view function, s32 lineNr);

        bool has_failed() const;
        bool has_succeeded() const;

        const std::string& error_message() const;

      private:
        HResult m_error;
        std::string m_error_message;
      };

      namespace internal
      {
        //-------------------------------------------------------------------------
        template <typename Func>
        HResult call_to_dx_api(Func func, std::string_view file, std::string_view function, s32 lineNr)
        {
          HResult hr = func();
          cera::renderer::directx::DXCall(hr, file, function, lineNr);
          return hr;
        }

        //-------------------------------------------------------------------------
        template <typename Func>
        bool call_to_dx_api_has_succeeded(Func func, std::string_view file, std::string_view function, s32 lineNr)
        {
          HResult hr = func();
          return cera::renderer::directx::DXCall(hr, file, function, lineNr).has_succeeded();
        }

        //-------------------------------------------------------------------------
        template <typename Func>
        bool call_to_dx_api_has_failed(Func func, std::string_view file, std::string_view function, s32 lineNr)
        {
          HResult hr = func();
          return cera::renderer::directx::DXCall(hr, file, function, lineNr).has_failed();
        }
      } // namespace internal
    } // namespace directx
  } // namespace renderer
} // namespace cera

#ifdef CERA_ENABLE_DX_CALL
  // This is just a wrapper around a dx call, it will log if something happened and return the result of the function call
  #define DX_CALL(function) cera::renderer::directx::internal::call_to_dx_api([&]() { return function; }, __FILE__, __FUNCTION__, __LINE__)

  // These two macros do exactly the same thing as DX_CALL the only difference is that it will check if it was successfull or not.
  // if (DX_FAILED(...))
  // {
  //     // Do some additional logic here
  // }
  #define DX_SUCCESS(function) cera::renderer::directx::internal::call_to_dx_api_has_succeeded([&]() { return function; }, __FILE__, __FUNCTION__, __LINE__)
  #define DX_FAILED(function)  cera::renderer::directx::internal::call_to_dx_api_has_failed([&]() { return function; }, __FILE__, __FUNCTION__, __LINE__)

#else

  #define CHECK_FOR_DX_ERRORS()

  #define DX_CALL(function)    function
  #define DX_SUCCESS(function) function
  #define DX_FAILED(function)  function

#endif