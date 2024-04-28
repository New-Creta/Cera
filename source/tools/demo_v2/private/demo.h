#pragma once

#include "generic_entrypoint.h"

namespace cera
{
    class demo : public abstract_game
    {
    public:
        demo();

    public:
        bool initialize() override;
        bool load_content() override;

        void on_update(const events::update_args& e) override;
        void on_render(const events::render_args& e) override;

        void unload_content() override;
        void destroy() override;
    };
}