#include "core/Application.h"
#include "core/ResourceManager.h"

#include <iostream>

int main(int argc, char* argv[]) try {
    (void)argc;

    // ResourceManager must be instantiated here -- DO NOT CHANGE
    ResourceManager::init(argv[0]);

    Application app;
    return app.run();

} catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Fatal error: unknown exception occurred" << std::endl;
    return EXIT_FAILURE;
}
