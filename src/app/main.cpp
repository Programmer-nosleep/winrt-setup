#include "app/application.h"

#include <winrt/base.h>

namespace {

struct ApartmentGuard {
    ApartmentGuard()
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
    }

    ~ApartmentGuard()
    {
        winrt::uninit_apartment();
    }
};

} // namespace

int main(int, char**)
{
    ApartmentGuard apartment_guard;
    Application application;
    return application.Run();
}
