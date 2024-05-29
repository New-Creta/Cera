#include "gui_renderer.h"

#include "generic_window.h"

namespace cera
{
    void gui_renderer::create_viewport(std::shared_ptr<generic_window> window)
    {
        if (m_window_viewport_map.find(window.get()) == std::cend(m_window_viewport_map))
        {
            s32 window_width = window->get_definition().

            // Clamp the window size to a reasonable default anything below 8 is a d3d warning and 8 is used anyway.
            // @todo Slate: This is a hack to work around menus being summoned with 0,0 for window size until they are ticked.
            int32 Width = FMath::Max(MIN_VIEWPORT_SIZE, FMath::CeilToInt(WindowSize.X));
            int32 Height = FMath::Max(MIN_VIEWPORT_SIZE, FMath::CeilToInt(WindowSize.Y));

            // Sanity check dimensions
            if (!ensureMsgf(Width <= MAX_VIEWPORT_SIZE && Height <= MAX_VIEWPORT_SIZE, TEXT("Invalid window with Width=%u and Height=%u"), Width, Height))
            {
                Width = FMath::Clamp(Width, MIN_VIEWPORT_SIZE, MAX_VIEWPORT_SIZE);
                Height = FMath::Clamp(Height, MIN_VIEWPORT_SIZE, MAX_VIEWPORT_SIZE);
            }

            FViewportInfo* NewInfo = new FViewportInfo();
            // Create Viewport RHI if it doesn't exist (this must be done on the game thread)
            TSharedRef<FGenericWindow> NativeWindow = Window->GetNativeWindow().ToSharedRef();
            NewInfo->OSWindow = NativeWindow->GetOSWindowHandle();
            NewInfo->Width = Width;
            NewInfo->Height = Height;
            NewInfo->DesiredWidth = Width;
            NewInfo->DesiredHeight = Height;
            NewInfo->ProjectionMatrix = CreateProjectionMatrix(Width, Height);
            // In MobileLDR case backbuffer format should match or be compatible with a SceneColor format in FSceneRenderTargets::GetDesiredMobileSceneColorFormat()
            if (bIsStandaloneStereoOnlyDevice || (GMaxRHIFeatureLevel == ERHIFeatureLevel::ES3_1 && !IsMobileHDR()))
            {
                NewInfo->PixelFormat = GetSlateRecommendedColorFormat();
            }
#if ALPHA_BLENDED_WINDOWS
            if (Window->GetTransparencySupport() == EWindowTransparency::PerPixel)
            {
                NewInfo->PixelFormat = GetSlateRecommendedColorFormat();
            }
#endif

            // SDR format holds the requested format in non HDR mode
            NewInfo->SDRPixelFormat = NewInfo->PixelFormat;
            HDRGetMetaData(NewInfo->HDRDisplayOutputFormat, NewInfo->HDRDisplayColorGamut, NewInfo->bSceneHDREnabled, Window->GetPositionInScreen(), Window->GetPositionInScreen() + Window->GetSizeInScreen(), NewInfo->OSWindow);

            if (NewInfo->bSceneHDREnabled)
            {
                NewInfo->PixelFormat = GRHIHDRDisplayOutputFormat;
            }

            // Sanity check dimensions
            checkf(Width <= MAX_VIEWPORT_SIZE && Height <= MAX_VIEWPORT_SIZE, TEXT("Invalid window with Width=%u and Height=%u"), Width, Height);

            bool bFullscreen = IsViewportFullscreen(*Window);
            NewInfo->ViewportRHI = RHICreateViewport(NewInfo->OSWindow, Width, Height, bFullscreen, NewInfo->PixelFormat);
            NewInfo->bFullscreen = bFullscreen;

            // Was the window created on a HDR compatible display?
            NewInfo->bHDREnabled = RHIGetColorSpace(NewInfo->ViewportRHI) != EColorSpaceAndEOTF::ERec709_sRGB;
            Window->SetIsHDR(NewInfo->bHDREnabled);

            WindowToViewportInfo.Add(&Window.Get(), NewInfo);

            BeginInitResource(NewInfo);
        }
    }

    void gui_renderer::on_window_destroyed(std::shared_ptr<generic_window> window)
    {

    }

    void gui_renderer::on_window_resized(std::shared_ptr<generic_window> window)
    {

    }
}