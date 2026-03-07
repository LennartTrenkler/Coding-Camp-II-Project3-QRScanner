#pragma once

#include <string>

// Opens a Windows file picker and returns the selected image path
// Returns an empty string if the dialog is cancelled or fails
std::string pickImageFile();