#include "generic_application.h"
#include "generic_application_message_handler.h"
#include "generic_window.h"

namespace cera
{
    generic_application::generic_application()
        :m_message_handler(std::make_shared<generic_application_message_handler>())
    {}
    
    generic_application::~generic_application() = default;

    void generic_application::set_message_handler(const std::shared_ptr<generic_application_message_handler> &in_message_handler)
    {
        m_message_handler = in_message_handler;
    }

    void generic_application::process_deferred_events()
    {
        // Nothing to implement
    }

    std::shared_ptr<generic_window> generic_application::make_window()
    {
        return std::make_shared<generic_window>();
    }

    void generic_application::initialize_window(const std::shared_ptr<generic_window>& window, const generic_window_definition& definition, const bool show)
    {
        // Nothing to implement
    }

    std::shared_ptr<generic_application_message_handler> generic_application::get_message_handler() const
    {
        return m_message_handler;
    }

    void generic_application::destroy_application()
    {
        
    }
}