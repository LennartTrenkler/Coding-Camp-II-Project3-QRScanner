#include "QRScanner.h"

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <iostream>
#include <string>
#include <cstdlib>

QRScanResult scanFromCamera(QRScanner& scanner) {
    QRScanResult result;

    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Could not open camera.\n";
        return result;
    }

    std::cout << "\nCamera opened successfully.\n";
    std::cout << "Press 'q' to quit camera scanning.\n";
    std::cout << "Hold a QR code in front of the camera.\n\n";

    cv::Mat frame;

    while (true) {
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "Failed to capture frame from camera.\n";
            break;
        }

        result = scanner.scanFrame(frame);

        cv::imshow("QR Scanner - Camera", result.annotatedImage);

        if (result.qrDetected) {
            std::cout << "QR code detected.\n";
            break;
        }

        char key = (char)cv::waitKey(1);
        if (key == 'q' || key == 'Q') {
            std::cout << "Camera scanning cancelled by user.\n";
            break;
        }
    }

    cap.release();
    cv::destroyWindow("QR Scanner - Camera");

    return result;
}

int main() {
    char choice = 'y';

    std::cout << "=====================================\n";
    std::cout << "           QR CODE SCANNER\n";
    std::cout << "=====================================\n";
    std::cout << "Live camera QR scanning\n\n";

    while (choice == 'y' || choice == 'Y') {
        QRScanner scanner;
        QRScanResult result = scanFromCamera(scanner);

        std::cout << "\n=====================================\n";
        std::cout << "              SCAN RESULT\n";
        std::cout << "=====================================\n";

        if (result.qrDetected) {
            std::cout << "Decoded QR Content:\n";
            std::cout << result.decodedText << "\n\n";

            if (result.decodedText.rfind("http", 0) == 0) {
                std::cout << "Detected content type: URL\n";

                char openLink;
                std::cout << "Do you want to open this link in your browser? (y/n): ";
                std::cin >> openLink;

                if (openLink == 'y' || openLink == 'Y') {
                    #ifdef _WIN32
                    std::string command = "start " + result.decodedText;
                    #else
                    std::string command = "open " + result.decodedText;
                    #endif
                    system(command.c_str());
                    std::cout << "Opening link in browser...\n";
                }
            } else {
                std::cout << "Detected content type: Text/Data\n";
            }
        } else {
            std::cout << "No QR code detected.\n";
        }

        if (!result.annotatedImage.empty()) {
            cv::imshow("QR Scanner", result.annotatedImage);
            cv::waitKey(0);
            cv::destroyAllWindows();
        }

        std::cout << "Do you want to scan again? (y/n): ";
        std::cin >> choice;
        std::cout << "\n";
    }

    std::cout << "Thank you for using the QR Code Scanner.\n";
    return 0;
}