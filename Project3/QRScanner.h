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
    void drawResult(cv::Mat& image,
                    const std::vector<cv::Point>& corners,
                    const std::string& decodedText) const;

    cv::QRCodeDetector detector;
};