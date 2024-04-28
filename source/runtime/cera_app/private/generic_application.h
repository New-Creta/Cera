#pragma once

#include <memory>

namespace cera
{
    class generic_window_definition;
    class generic_application_message_handler;
    class generic_window;
    
    class generic_application
    {
    public:
        generic_application();
        virtual ~generic_application();

        void set_message_handler(const std::shared_ptr<generic_application_message_handler>& in_message_handler);

        virtual void process_deferred_events();
        virtual std::shared_ptr<generic_window> make_window();
        virtual void initialize_window(const std::shared_ptr<generic_window>& window, const generic_window_definition& definition, const bool show);
        virtual void destroy_application();

    protected:
        std::shared_ptr<generic_application_message_handler> get_message_handler() const;

    private:
        std::shared_ptr<generic_application_message_handler> m_message_handler;
    };
}