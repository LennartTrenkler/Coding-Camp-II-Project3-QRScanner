#include "QRScanner.h"
#include "FileDialog.h"

#include <opencv2/highgui.hpp>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::string imagePath;

    // Use the command line path if one was passed in
    if (argc >= 2) {
        imagePath = argv[1];
    }
    else {
        // Otherwise open the Windows file picker
        imagePath = pickImageFile();
    }

    // Stop if no file was selected
    if (imagePath.empty()) {
        std::cout << "No image selected.\n";
        return 0;
    }

    QRScanner scanner;
    QRScanResult result = scanner.scanImage(imagePath);

    // Stop if the image could not be loaded
    if (!result.imageLoaded) {
        std::cerr << "Could not open image: " << imagePath << '\n';
        return 1;
    }

    // Print the decoded result in the console
    if (result.qrDetected) {
        std::cout << "Decoded QR content: " << result.decodedText << '\n';
    }
    else {
        std::cout << "No QR code detected.\n";
    }

    // Show the annotated image
    cv::imshow("QR Scanner MVP", result.annotatedImage);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}