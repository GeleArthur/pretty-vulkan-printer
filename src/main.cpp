#include <iostream>
#include "App.h"

int main()
{
    try
    {
        App{}.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}