#include "QRScanner.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>

QRScanResult QRScanner::scanImage(const std::string& imagePath) {
    QRScanResult result;

    // Load the image from the given path
    result.originalImage = cv::imread(imagePath);
    result.imageLoaded = !result.originalImage.empty();

    if (!result.imageLoaded) {
        return result;
    }

    // Reuse the frame-based scan path so camera input can use the same logic later
    return scanFrame(result.originalImage);
}

QRScanResult QRScanner::scanFrame(const cv::Mat& frame) {
    QRScanResult result;

    // Stop early if the frame is empty
    if (frame.empty()) {
        return result;
    }

    // Store copies of the frame for processing and display
    result.imageLoaded = true;
    result.originalImage = frame.clone();
    result.annotatedImage = frame.clone();

    // Detect and decode one QR code
    cv::Mat bbox;
    result.decodedText = detector.detectAndDecode(result.originalImage, bbox);

    // Convert the detected box into corner points
    result.corners = extractCorners(bbox);
    result.qrDetected = !result.decodedText.empty() && result.corners.size() == 4;

    // Draw the visual result on the output image
    drawResult(result.annotatedImage, result.corners, result.decodedText);

    return result;
}

std::vector<cv::Point> QRScanner::extractCorners(const cv::Mat& bbox) const {
    std::vector<cv::Point> corners;

    // Return no points if OpenCV did not provide a valid box
    if (bbox.empty() || bbox.total() < 4) {
        return corners;
    }

    // Reshape the box data into 4 points
    cv::Mat reshaped = bbox.reshape(2, 4);

    for (int i = 0; i < 4; ++i) {
        const cv::Point2f point = reshaped.at<cv::Point2f>(i, 0);
        corners.emplace_back(
            static_cast<int>(std::round(point.x)),
            static_cast<int>(std::round(point.y))
        );
    }

    return corners;
}

void QRScanner::drawResult(cv::Mat& image,
                           const std::vector<cv::Point>& corners,
                           const std::string& decodedText) const {
    // Draw the box if 4 valid corners are available
    if (corners.size() == 4) {
        for (int i = 0; i < 4; ++i) {
            const cv::Point& p1 = corners[i];
            const cv::Point& p2 = corners[(i + 1) % 4];
            cv::line(image, p1, p2, cv::Scalar(0, 255, 0), 2);
        }
    }

    // Show decoded text or a fallback message
    std::string label;
    cv::Scalar color;

    if (!decodedText.empty()) {
        label = "Decoded: " + decodedText;
        color = cv::Scalar(0, 255, 0);
    } else {
        label = "No QR code detected";
        color = cv::Scalar(0, 0, 255);
    }

    // Keep the label short enough for the window
    if (label.size() > 60) {
        label = label.substr(0, 60) + "...";
    }

    cv::putText(image,
                label,
                cv::Point(20, image.rows - 20),
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                color,
                2);
}