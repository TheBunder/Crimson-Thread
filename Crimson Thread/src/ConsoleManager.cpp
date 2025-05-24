#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "include/ConsoleManager.h"
#include "include/utils.h"
#include "include/Visualizer.h"

// Function to get the console window height in character rows
int GetConsoleWindowHeight() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        PrintError("Error: Could not get console handle in GetConsoleWindowHeight. Code: %lu\n", GetLastError());
        return -1; // Indicate an error
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        PrintError("Error: Could not get console screen buffer info in GetConsoleWindowHeight. Code: %lu\n",
                GetLastError());
        return -1; // Indicate an error
    }

    // Calculate height (height start from zero)
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

// Function to get the console window width in character columns
int GetConsoleWindowWidth() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        PrintError("Error: Could not get console handle in GetConsoleWindowWidth. Code: %lu\n", GetLastError());
        return -1;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        PrintError("Error: Could not get console screen buffer info in GetConsoleWindowWidth. Code: %lu\n",
                GetLastError());
        return -1;
    }

    // Calculate width (width start from zero)
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

// Function to simulate pressing Ctrl and Minus for zooming out
void SimulateCtrlMinusZoomOut() {
    INPUT inputs[4] = {};
    int i = 0;

    // Ctrl down
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_CONTROL;
    i++;

    // '-' key down
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_OEM_MINUS; // Use VK_OEM_MINUS for the minus key
    i++;

    // '-' key up
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_OEM_MINUS;
    inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    i++;

    // Ctrl up
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_CONTROL;
    inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    i++;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

// Function to simulate pressing Ctrl and Plus for zooming in
void SimulateCtrlPlusZoomIn() {
    INPUT inputs[4] = {};
    int i = 0;

    // Ctrl down
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_CONTROL;
    i++;

    // '+' key down
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_OEM_PLUS; // Use VK_OEM_PLUS for the plus key
    i++;

    // '+' key up
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_OEM_PLUS;
    inputs[i].ki.dwFlags = KEYEVENTF_KEYUP; // Indicate key release
    i++;

    // Ctrl up
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_CONTROL;
    inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;
    i++;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

// Simulate pressing the F11 key
void SimulateF11Fullscreen() {
    INPUT inputs[2] = {};
    int i = 0;

    // F11 down
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_F11; // Key code of F11
    i++;

    // F11 up
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_F11;
    inputs[i].ki.dwFlags = KEYEVENTF_KEYUP; // Key release
    i++;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

bool GetCurrentConsoleDimensions(int& height, int& width) {
    height = GetConsoleWindowHeight();
    width = GetConsoleWindowWidth();
    if (height == -1 || width == -1) {
        PrintError("Error: Could not retrieve current console dimensions.\n");
        return false;
    }
    return true;
}

// Function to ensure the console is in fullscreen by simulating F11.
bool MakeFullScreen() {
    const std::chrono::milliseconds sleepDurationAfterKeySim(50); // Time to wait after simulating F11
    const int tolerance = 5; // The amount off negative change we are willing to look over

    // Get initial height before the first F11 press
    int initialHeight,initialWidth;
    if (!GetCurrentConsoleDimensions(initialHeight,initialWidth)) {
        return false; // Exit the function if we can't check dimensions
    }

    // Do the first F11
    SimulateF11Fullscreen();
    std::this_thread::sleep_for(sleepDurationAfterKeySim); // Give the terminal time to process change

    // Get height after the first F11 press
    int heightAfterFirstF11, widthAfterFirstF11;
    if (!GetCurrentConsoleDimensions(heightAfterFirstF11,widthAfterFirstF11)) {
        return false; // Exit the function if we can't check dimensions
    }

    // If one of the dimensions significantly decreased, it means F11 exited the full screen
    if (heightAfterFirstF11 < initialHeight - tolerance || widthAfterFirstF11 < initialWidth - tolerance) {
        SimulateF11Fullscreen();
        // Give the terminal time to process the second F11 key press
        std::this_thread::sleep_for(sleepDurationAfterKeySim);
    }

    return true;
}

// Zoom in as much as possible whill making sure the grid fit.
bool ZoomInToEnlarge(int targetHeight, int targetWidth, int tolerance, const int maxAttempts,
                     const std::chrono::milliseconds sleepDuration) {
    int currentHeight, currentWidth;
    int attempts = 0;

    // Check if the console dimensions are available.
    if (!GetCurrentConsoleDimensions(currentHeight, currentWidth)) {
        return false; // If we can't get the dimensions, we can't adjust.
    }

    while (!(currentHeight < targetHeight - tolerance || currentWidth < targetWidth - tolerance)
           && attempts < maxAttempts) {
        if (abs(currentHeight - targetHeight) < tolerance && abs(currentWidth - targetWidth) < tolerance) {
            return true;
        }

        SimulateCtrlPlusZoomIn();

        std::this_thread::sleep_for(sleepDuration); // Wait for the terminal to react
        if (!GetCurrentConsoleDimensions(currentHeight, currentWidth)) {
            return false; // If we can't get the dimensions, we can't adjust.
        }
        attempts++;
    }

    if (!(attempts < maxAttempts)) {
        PrintWarning("Warning: ZoomInToEnlarge did not reach target within attempts.\n");
        return false;
    }

    return true;
}

// Zoom out as much as need to make the grid fit.
bool ZoomOutToFit(int targetHeight, int targetWidth, int tolerance, const int maxAttempts,
                     const std::chrono::milliseconds sleepDuration) {
    int currentHeight, currentWidth;
    int attempts = 0;

    // Check if the console dimensions are available.
    if (!GetCurrentConsoleDimensions(currentHeight, currentWidth)) {
        return false; // If we can't get the dimensions, we can't adjust.
    }

    while ((currentHeight < targetHeight - tolerance || currentWidth < targetWidth - tolerance)
           && attempts < maxAttempts) {
        if (abs(currentHeight - targetHeight) < tolerance && abs(currentWidth - targetWidth) < tolerance) {
            return true;
        }

        SimulateCtrlMinusZoomOut();

        std::this_thread::sleep_for(sleepDuration); // Wait for the terminal to react
        if (!GetCurrentConsoleDimensions(currentHeight, currentWidth)) {
            return false; // If we can't get the dimensions, we can't adjust.
        }
        attempts++;
    }

    if (!(attempts < maxAttempts)) {
        PrintWarning("Warning: ZoomOutToFit did not reach target within attempts.\n");
        return false;
    }

    return true;
}

void SetConsole() {
    // Make the console as big as it can using fullscreen initiated by simulating F11
    if (!MakeFullScreen()) {
        PrintWarning("Warning: Could not make console fullscreen.\n");
    }

    const int targetHeight = GRID_HEIGHT + 4;
    const int targetWidth = GRID_WIDTH + 10;
    const int tolerance = 2; // Allow height to be within +/- tolerance of the target
    const int maxAttempts = 100; // Prevent infinite loops
    const std::chrono::milliseconds sleepDuration(50); // Time to wait between zoom steps to ensure the console refresh

    printf("Attempting to adjust console height to fit the grid.\n", targetHeight);

    // make the grid as big as it can and still\ close to fit.
    bool success =ZoomInToEnlarge(targetHeight, targetWidth, tolerance, maxAttempts, sleepDuration);

    // Make the grid as small as it needs to fit. We try to do step 2 even if step 1 failed.
    success &= ZoomOutToFit(targetHeight, targetWidth, tolerance, maxAttempts, sleepDuration);

    if (success) {
        printf("Adjustment completed successfully. Please don't revert the adjustment.\n");
    }else {
        PrintWarning("Console adjustment failed or did not reach optimal size. Display might not be optimal.\n");
    }
}