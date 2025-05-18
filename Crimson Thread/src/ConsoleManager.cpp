#include "include/ConsoleManager.h"
#include "include/utils.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

// Function to get the console window height in character rows
int GetConsoleWindowHeight() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        // Use fprintf to stderr for error messages
        fprintf(stderr, "Error: Could not get console handle in GetConsoleWindowHeight. Code: %lu\n", GetLastError());
        return -1; // Indicate an error
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        // Use fprintf to stderr for error messages
        fprintf(stderr, "Error: Could not get console screen buffer info in GetConsoleWindowHeight. Code: %lu\n", GetLastError());
        return -1; // Indicate an error
    }

    // Calculate height from the visible window rectangle
    // srWindow.Bottom and srWindow.Top are 0-based, so we add 1 for the total number of rows
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
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

// Function to simulate pressing the F11 key
void SimulateF11Fullscreen() {
    INPUT inputs[2] = {};
    int i = 0;

    // F11 down
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_F11; // Virtual key code for F11
    i++;

    // F11 up
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki.wVk = VK_F11;
    inputs[i].ki.dwFlags = KEYEVENTF_KEYUP; // Indicate key release
    i++;

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

//  Enables ANSI escape code processing on Windows console.
// Returns true if successful, false otherwise.
bool EnableAnsiEscapeCodes() {
    // Get a handle to the standard output device (the console)
    HANDLE stdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdOutputHandle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error getting standard output handle.\n");
        return false;
    }

    // Get the current console mode flags
    DWORD consoleMode = 0;
    if (!GetConsoleMode(stdOutputHandle, &consoleMode)) {
        fprintf(stderr, "Error getting console mode.\n");
        return false;
    }

    // Check if ANSI processing is already enabled
    if (consoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
        // Already enabled, nothing to do
        return true;
    }

    // Set the ENABLE_VIRTUAL_TERMINAL_PROCESSING flag
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Set the new console mode
    if (!SetConsoleMode(stdOutputHandle, consoleMode)) {
        fprintf(stderr, "Error setting console mode. This may require Windows 10 or newer.\n");
        return false;
    }

    return true;
}

// Function to ensure the console is in fullscreen
// by simulating F11 and checking if it successfully toggled or remained fullscreen.
void MakeFullScreen() {
    // Constants specific to this F11 toggling logic
    const std::chrono::milliseconds sleepDurationAfterKeySim(50); // Time to wait after simulating F11
    const int height_change_tolerance = 3; // If height changes by less than this many rows after F11, assume it didn't exit fullscreen

    // Get initial height before first F11 press
    int initialHeight = GetConsoleWindowHeight();
    if (initialHeight == -1) {
         printf("Error getting initial console height. Cannot reliably perform F11 check.\n");
         return; // Exit the function if we can't check height
    }

    SimulateF11Fullscreen();
    std::this_thread::sleep_for(sleepDurationAfterKeySim); // Give the terminal time to process change

    // Get height after the first F11 press
    int heightAfterFirstF11 = GetConsoleWindowHeight();

    // If the height significantly decreased, it means F11 likely toggled fullscreen OFF
    // We also check heightAfterFirstF11 != -1 in case the get height call failed after simulation
    if (heightAfterFirstF11 != -1 && heightAfterFirstF11 < initialHeight - height_change_tolerance) {
         SimulateF11Fullscreen();
         // Give the terminal time to process the second F11 key press
         std::this_thread::sleep_for(sleepDurationAfterKeySim);
    }
}

void SetConsole() {
    EnableAnsiEscapeCodes(); // Used to enable ansi escape codes

    // Ensure console is fullscreen using the F11 toggle logic
    MakeFullScreen();

    const int targetHeight = GRID_HEIGHT + 10;
    const int tolerance = 5; // Allow height to be within +/- tolerance of the target
    const int maxAttempts = 100; // Prevent infinite loops
    const std::chrono::milliseconds sleepDuration(50); // Time to wait between zoom steps

    printf("Attempting to adjust console height to fit the grid.",targetHeight);

    int currentHeight = GetConsoleWindowHeight();
    int attempts = 0;

    while (abs(currentHeight - targetHeight) > tolerance && attempts < maxAttempts) {
        if (currentHeight == -1) {
            printf("Error getting console height. Aborting.");
            break;
        }

        if (currentHeight < targetHeight) {
            // printf("Current height (%d) is less than target (%d). Zooming out...", currentHeight, targetHeight);
            SimulateCtrlMinusZoomOut();
        } else { // currentHeight > targetHeight
            // printf("Current height (%d) is greater than target (%d). Zooming in...", currentHeight, targetHeight);
            SimulateCtrlPlusZoomIn();
        }

        std::this_thread::sleep_for(sleepDuration); // Wait for the terminal to react
        currentHeight = GetConsoleWindowHeight();
        attempts++;
    }
}

