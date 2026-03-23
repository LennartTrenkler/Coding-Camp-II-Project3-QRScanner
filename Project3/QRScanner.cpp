#include "QRScanner.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>
#include <sstream>
#include <vector>

namespace {

    // Returns the rendered width of one text string
    int getRenderedTextWidth(const std::string& text,
        int fontFace,
        double fontScale,
        int thickness) {
        int baseline = 0;
        return cv::getTextSize(text, fontFace, fontScale, thickness, &baseline).width;
    }

    // Splits one long token into smaller pieces if it does not fit in one line
    std::vector<std::string> splitLongToken(const std::string& token,
        int maxWidth,
        int fontFace,
        double fontScale,
        int thickness) {
        std::vector<std::string> parts;
        std::string current;

        for (char ch : token) {
            std::string candidate = current + ch;

            if (current.empty() ||
                getRenderedTextWidth(candidate, fontFace, fontScale, thickness) <= maxWidth) {
                current = candidate;
            }
            else {
                parts.push_back(current);
                current = std::string(1, ch);
            }
        }

        if (!current.empty()) {
            parts.push_back(current);
        }

        return parts;
    }

    // Wraps text into multiple lines that fit the available width
    std::vector<std::string> wrapText(const std::string& text,
        int maxWidth,
        int fontFace,
        double fontScale,
        int thickness) {
        std::vector<std::string> lines;
        std::istringstream stream(text);
        std::string token;
        std::string currentLine;

        while (stream >> token) {
            std::vector<std::string> tokenParts;

            if (getRenderedTextWidth(token, fontFace, fontScale, thickness) <= maxWidth) {
                tokenParts.push_back(token);
            }
            else {
                tokenParts = splitLongToken(token, maxWidth, fontFace, fontScale, thickness);
            }

            for (const std::string& part : tokenParts) {
                if (currentLine.empty()) {
                    currentLine = part;
                }
                else {
                    std::string candidate = currentLine + " " + part;

                    if (getRenderedTextWidth(candidate, fontFace, fontScale, thickness) <= maxWidth) {
                        currentLine = candidate;
                    }
                    else {
                        lines.push_back(currentLine);
                        currentLine = part;
                    }
                }
            }
        }

        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }

        if (lines.empty()) {
            lines.push_back("");
        }

        return lines;
    }

} // namespace

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
    // Use a calmer visual style than the bright green version
    const cv::Scalar boxColor(190, 120, 60);       // muted blue
    const cv::Scalar panelColor(34, 34, 34);       // dark gray
    const cv::Scalar separatorColor(70, 70, 70);   // medium gray
    const cv::Scalar textColor(245, 245, 245);     // near white
    const cv::Scalar warningTextColor(140, 140, 230);

    const int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    const double fontScale = 0.62;
    const int thickness = 1;
    const int sidePadding = 18;
    const int topPadding = 16;
    const int bottomPadding = 16;
    const int lineSpacing = 10;

    // Draw the QR border if valid corners are available
    if (corners.size() == 4) {
        for (int i = 0; i < 4; ++i) {
            const cv::Point& p1 = corners[i];
            const cv::Point& p2 = corners[(i + 1) % 4];
            cv::line(image, p1, p2, boxColor, 2);
        }
    }

    // Build the full label text without truncating it
    std::string label;

    if (!decodedText.empty()) {
        label = "Decoded: " + decodedText;
    }
    else {
        label = "No QR code detected";
    }

    const int maxTextWidth = std::max(120, image.cols - (sidePadding * 2));
    const std::vector<std::string> lines =
        wrapText(label, maxTextWidth, fontFace, fontScale, thickness);

    int baseline = 0;
    const int textHeight = cv::getTextSize("Ag", fontFace, fontScale, thickness, &baseline).height;
    const int lineStep = textHeight + lineSpacing;
    const int panelHeight = topPadding + static_cast<int>(lines.size()) * lineStep + bottomPadding;

    // Extend the image with a text panel at the bottom
    cv::Mat output(
        image.rows + panelHeight,
        image.cols,
        image.type(),
        cv::Scalar(255, 255, 255)
    );

    image.copyTo(output(cv::Rect(0, 0, image.cols, image.rows)));

    cv::rectangle(
        output,
        cv::Rect(0, image.rows, image.cols, panelHeight),
        panelColor,
        cv::FILLED
    );

    cv::line(
        output,
        cv::Point(0, image.rows),
        cv::Point(image.cols, image.rows),
        separatorColor,
        1
    );

    const cv::Scalar finalTextColor = decodedText.empty() ? warningTextColor : textColor;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const int y = image.rows + topPadding + textHeight + static_cast<int>(i) * lineStep;

        cv::putText(
            output,
            lines[i],
            cv::Point(sidePadding, y),
            fontFace,
            fontScale,
            finalTextColor,
            thickness,
            cv::LINE_AA
        );
    }

    image = output;
}