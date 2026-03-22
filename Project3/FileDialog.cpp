#include "FileDialog.h"

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h>
#endif

#include <iostream>
#include <string>

#ifdef _WIN32

// Converts a UTF-16 Windows string into UTF-8 for OpenCV
static std::string wideToUtf8(const std::wstring& wideText) {
    if (wideText.empty()) {
        return "";
    }

    const int sizeNeeded = WideCharToMultiByte(
        CP_UTF8,
        0,
        wideText.c_str(),
        static_cast<int>(wideText.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (sizeNeeded <= 0) {
        return "";
    }

    std::string utf8Text(sizeNeeded, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wideText.c_str(),
        static_cast<int>(wideText.size()),
        &utf8Text[0],
        sizeNeeded,
        nullptr,
        nullptr
    );

    return utf8Text;
}

std::string pickImageFile() {
    std::string selectedPath;

    // Initialize COM for the dialog call
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool comReady = SUCCEEDED(hr);

    if (!comReady) {
        return "";
    }

    IFileOpenDialog* fileDialog = nullptr;

    // Create the Windows open-file dialog object
    hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&fileDialog)
    );

    if (SUCCEEDED(hr)) {
        // Limit the dialog to common image types
        const COMDLG_FILTERSPEC fileTypes[] = {
            { L"Image Files", L"*.png;*.jpg;*.jpeg;*.bmp" },
            { L"PNG Files",   L"*.png" },
            { L"JPEG Files",  L"*.jpg;*.jpeg" },
            { L"Bitmap Files",L"*.bmp" },
            { L"All Files",   L"*.*" }
        };

        fileDialog->SetFileTypes(
            static_cast<UINT>(sizeof(fileTypes) / sizeof(fileTypes[0])),
            fileTypes
        );
        fileDialog->SetFileTypeIndex(1);
        fileDialog->SetTitle(L"Select an image with a QR code");

        // Show the dialog and wait for user input
        hr = fileDialog->Show(nullptr);

        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            hr = fileDialog->GetResult(&item);

            if (SUCCEEDED(hr)) {
                PWSTR filePath = nullptr;
                hr = item->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

                if (SUCCEEDED(hr) && filePath != nullptr) {
                    selectedPath = wideToUtf8(filePath);
                    CoTaskMemFree(filePath);
                }

                item->Release();
            }
        }

        fileDialog->Release();
    }

    CoUninitialize();
    return selectedPath;
}

#else

std::string pickImageFile() {
    std::string selectedPath;

    // Use a simple path prompt on non-Windows systems for now
    std::cout << "Enter the full path to the image file: ";
    std::getline(std::cin >> std::ws, selectedPath);

    return selectedPath;
}

#endif