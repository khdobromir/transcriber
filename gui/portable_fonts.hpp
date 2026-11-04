#pragma once
// Linux GUI startup only; must run before QGuiApplication and Qt font discovery.
bool initializePortableFonts();
void writePortableFontDiagnostics();
