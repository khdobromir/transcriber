#include "portable_fonts.hpp"
#include <QGuiApplication>
#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    try {
        const bool skip = argc > 1 && std::string_view(argv[1]) == "--skip-init";
        if (!skip) initializePortableFonts();
        QGuiApplication app(argc, argv);
        writePortableFontDiagnostics();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
