#pragma once

#include <string>

// Opens image selection for the current platform
// Windows uses a native file dialog
// Other platforms use a console path prompt for now
std::string pickImageFile();