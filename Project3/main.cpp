#include "QRScanner.h"
#include "FileDialog.h"

#include <opencv2/highgui.hpp>

#include <iostream>
#include <string>
#include <cstdlib>   // needed for opening browser

int main(int argc, char** argv) {

    char choice = 'y';

    std::cout << "=====================================\n";
    std::cout << "           QR CODE SCANNER\n";
    std::cout << "=====================================\n";
    std::cout << "Select an image containing a QR code.\n";
    std::cout << "The program will decode and display the result.\n\n";

    while (choice == 'y' || choice == 'Y') {

        std::string imagePath;

        // Use command line path if provided
        if (argc >= 2) {
            imagePath = argv[1];
        }
        else {
            imagePath = pickImageFile();
        }

        if (imagePath.empty()) {
            std::cout << "No image selected.\n";
            return 0;
        }

        std::cout << "\nProcessing image...\n";

        QRScanner scanner;
        QRScanResult result = scanner.scanImage(imagePath);

        if (!result.imageLoaded) {
            std::cerr << "Could not open image: " << imagePath << '\n';
            return 1;
        }

        std::cout << "\n=====================================\n";
        std::cout << "              SCAN RESULT\n";
        std::cout << "=====================================\n";

        if (result.qrDetected) {

            std::cout << "Decoded QR Content:\n";
            std::cout << result.decodedText << "\n\n";

            // Detect if QR contains a URL
            if (result.decodedText.rfind("http", 0) == 0) {

                std::cout << "Detected content type: URL\n";

                char openLink;
                std::cout << "Do you want to open this link in your browser? (y/n): ";
                std::cin >> openLink;

                if (openLink == 'y' || openLink == 'Y') {

                    std::string command = "start " + result.decodedText;
                    system(command.c_str());

                    std::cout << "Opening link in browser...\n";
                }
            }
            else {
                std::cout << "Detected content type: Text/Data\n";
            }

        }
        else {
            std::cout << "No QR code detected in the image.\n";
            std::cout << "Try using a clearer image.\n";
        }

        std::cout << "=====================================\n\n";

        // Show the annotated image
        cv::imshow("QR Scanner", result.annotatedImage);
        cv::waitKey(0);
        cv::destroyAllWindows();

        std::cout << "Do you want to scan another image? (y/n): ";
        std::cin >> choice;

        std::cout << "\n";
    }

    std::cout << "Thank you for using the QR Code Scanner.\n";

    return 0;
}
























