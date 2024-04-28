#pragma once

#include <memory>

namespace cera
{
    class generic_application;
    
    namespace windows
    {
        std::shared_ptr<generic_application> create_application();

        void pump_messages(bool from_main_loop);
    }
}