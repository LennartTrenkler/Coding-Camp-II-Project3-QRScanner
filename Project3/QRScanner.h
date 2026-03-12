#pragma once

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

#include <string>
#include <vector>

// Stores the result of one QR scan
struct QRScanResult {
    bool imageLoaded = false;
    bool qrDetected = false;
    std::string decodedText;
    cv::Mat originalImage;
    cv::Mat annotatedImage;
    std::vector<cv::Point> corners;
};

class QRScanner {
public:
    // Loads an image from disk and scans it
    QRScanResult scanImage(const std::string& imagePath);

    // Scans an already available frame
    QRScanResult scanFrame(const cv::Mat& frame);

private:
    // Converts the OpenCV bounding box into 4 corner points
    std::vector<cv::Point> extractCorners(const cv::Mat& bbox) const;

    // Draws the result on the image
void QRScanner::drawResult(cv::Mat& image,
                           const std::vector<cv::Point>& corners,
                           const std::string& decodedText) const {

    if (corners.size() == 4) {

        // Draw green box around QR code
        for (int i = 0; i < 4; ++i) {
            const cv::Point& p1 = corners[i];
            const cv::Point& p2 = corners[(i + 1) % 4];
            cv::line(image, p1, p2, cv::Scalar(0, 255, 0), 3);
        }

        // Compute center point of QR code
        cv::Point center(
            (corners[0].x + corners[2].x) / 2,
            (corners[0].y + corners[2].y) / 2
        );

        // Draw a center marker
        cv::circle(image, center, 6, cv::Scalar(0, 255, 0), -1);
    }

    std::string label;
    cv::Scalar color;

    if (!decodedText.empty()) {
        label = "QR Detected: " + decodedText;
        color = cv::Scalar(0, 255, 0);
    } else {
        label = "No QR Code Found";
        color = cv::Scalar(0, 0, 255);
    }

    // Limit label size so it fits the window
    if (label.size() > 60) {
        label = label.substr(0, 60) + "...";
    }

    // Draw label background
    cv::rectangle(image,
                  cv::Point(15, image.rows - 50),
                  cv::Point(600, image.rows - 10),
                  cv::Scalar(0, 0, 0),
                  -1);

    // Draw text label
    cv::putText(image,
                label,
                cv::Point(20, image.rows - 20),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                color,
                2);
}
