#pragma once

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

#include <string>
#include <vector>

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
    QRScanResult scanImage(const std::string& imagePath);
    QRScanResult scanFrame(const cv::Mat& frame);

private:
    std::vector<cv::Point> extractCorners(const cv::Mat& bbox) const;

    void drawResult(cv::Mat& image,
                    const std::vector<cv::Point>& corners,
                    const std::string& decodedText) const;

    cv::QRCodeDetector detector;
};