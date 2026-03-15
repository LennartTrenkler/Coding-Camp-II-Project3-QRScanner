#include "QRScanner.h"

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <iostream>
#include <string>
#include <cstdlib>   // needed for opening browser

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

        // If a QR code is detected, stop scanning
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

int main(int argc, char** argv) {

    char choice = 'y';
    char mode;

    std::cout << "=====================================\n";
    std::cout << "           QR CODE SCANNER\n";
    std::cout << "=====================================\n";
    std::cout << "Choose whether to scan from an image or from the camera.\n";
    std::cout << "The program will decode and display the result.\n\n";

    while (choice == 'y' || choice == 'Y') {

        QRScanner scanner;
        QRScanResult result;

        std::cout << "Choose input mode:\n";
        std::cout << "1 = Image file\n";
        std::cout << "2 = Camera\n";
        std::cout << "Your choice: ";
        std::cin >> mode;

        if (mode == '1') {
            std::string imagePath;

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
            result = scanner.scanImage(imagePath);

            if (!result.imageLoaded) {
                std::cerr << "Could not open image: " << imagePath << '\n';
                return 1;
            }
        }
        else if (mode == '2') {
            std::cout << "\nStarting camera scanner...\n";
            result = scanFromCamera(scanner);
        }
        else {
            std::cout << "Invalid choice.\n";
            return 0;
        }
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

                    std::string command = "open " + result.decodedText;
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
























