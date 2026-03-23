#include "QRScanner.h"
#include "FileDialog.h"

#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <iostream>
#include <string>
#include <cstdlib>

// Checks whether the decoded text looks like a web URL
bool isUrl(const std::string& text) {
    return text.rfind("http://", 0) == 0 || text.rfind("https://", 0) == 0;
}

// Opens a URL in the default browser for the current platform
bool openUrlInBrowser(const std::string& url) {
#ifdef _WIN32
    std::string command = "start \"\" \"" + url + "\"";
    return std::system(command.c_str()) == 0;
#elif defined(__APPLE__)
    std::string command = "open \"" + url + "\"";
    return std::system(command.c_str()) == 0;
#elif defined(__linux__)
    std::string command = "xdg-open \"" + url + "\"";
    return std::system(command.c_str()) == 0;
#else
    (void)url;
    return false;
#endif
}

// Scans frames from the default camera until a QR code is found or the user quits
QRScanResult scanFromCamera(QRScanner& scanner) {
    QRScanResult result;

    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Could not open camera.\n";
        return result;
    }

    cv::namedWindow("QR Scanner - Camera", cv::WINDOW_NORMAL);
    cv::resizeWindow("QR Scanner - Camera", 900, 700);

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

        if (!result.annotatedImage.empty()) {
            cv::imshow("QR Scanner - Camera", result.annotatedImage);
        }

        // Stop scanning as soon as a QR code is detected
        if (result.qrDetected) {
            std::cout << "QR code detected.\n";
            break;
        }

        char key = static_cast<char>(cv::waitKey(1));

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
    bool usedCommandLinePath = false;

    std::cout << "=====================================\n";
    std::cout << "           QR CODE SCANNER\n";
    std::cout << "=====================================\n";
    std::cout << "Choose whether to scan from an image or from the camera.\n";
    std::cout << "The program will decode and display the result.\n\n";

    while (choice == 'y' || choice == 'Y') {
        QRScanner scanner;
        QRScanResult result;
        char mode;

        std::cout << "Choose input mode:\n";
        std::cout << "1 = Image file\n";
        std::cout << "2 = Camera\n";
        std::cout << "Your choice: ";
        std::cin >> mode;

        if (mode == '1') {
            std::string imagePath;

            // Use the command line path only once
            if (argc >= 2 && !usedCommandLinePath) {
                imagePath = argv[1];
                usedCommandLinePath = true;
            }
            else {
                imagePath = pickImageFile();
            }

            if (imagePath.empty()) {
                std::cout << "No image selected.\n";
                std::cout << "Do you want to try again? (y/n): ";
                std::cin >> choice;
                std::cout << "\n";
                continue;
            }

            std::cout << "\nProcessing image...\n";
            result = scanner.scanImage(imagePath);

            if (!result.imageLoaded) {
                std::cerr << "Could not open image: " << imagePath << '\n';
                std::cout << "Do you want to try again? (y/n): ";
                std::cin >> choice;
                std::cout << "\n";
                continue;
            }
        }
        else if (mode == '2') {
            std::cout << "\nStarting camera scanner...\n";
            result = scanFromCamera(scanner);

            if (!result.imageLoaded) {
                std::cout << "Camera scan did not produce a usable frame.\n";
                std::cout << "Do you want to try again? (y/n): ";
                std::cin >> choice;
                std::cout << "\n";
                continue;
            }
        }
        else {
            std::cout << "Invalid choice.\n";
            std::cout << "Do you want to try again? (y/n): ";
            std::cin >> choice;
            std::cout << "\n";
            continue;
        }

        std::cout << "\n=====================================\n";
        std::cout << "              SCAN RESULT\n";
        std::cout << "=====================================\n";

        if (result.qrDetected) {
            std::cout << "Decoded QR Content:\n";
            std::cout << result.decodedText << "\n\n";

            if (isUrl(result.decodedText)) {
                std::cout << "Detected content type: URL\n";

                char openLink;
                std::cout << "Do you want to open this link in your browser? (y/n): ";
                std::cin >> openLink;

                if (openLink == 'y' || openLink == 'Y') {
                    if (openUrlInBrowser(result.decodedText)) {
                        std::cout << "Opening link in browser...\n";
                    }
                    else {
                        std::cout << "Could not open the link on this system.\n";
                    }
                }
            }
            else {
                std::cout << "Detected content type: Text/Data\n";
            }
        }
        else {
            std::cout << "No QR code detected.\n";
            std::cout << "Try using a clearer image or a more stable camera view.\n";
        }

        std::cout << "=====================================\n\n";

        // Show the annotated result if available
        if (!result.annotatedImage.empty()) {
            cv::namedWindow("QR Scanner", cv::WINDOW_NORMAL);
            cv::resizeWindow("QR Scanner", 900, 700);
            cv::imshow("QR Scanner", result.annotatedImage);
            cv::waitKey(0);
            cv::destroyAllWindows();
        }

        std::cout << "Do you want to scan another image? (y/n): ";
        std::cin >> choice;
        std::cout << "\n";
    }

    std::cout << "Thank you for using the QR Code Scanner.\n";

    return 0;
}