#pragma once

#include "abstract_game.h"

#include <functional>
#include <memory>
#include <string>

namespace cera
{
    struct generic_application_creation_params
    {
        std::shared_ptr<abstract_game> game = nullptr;

        s32 game_window_width = 1280;
        s32 game_window_height = 720;
    };
}