#include <opencv2/opencv.hpp>

#include "observer/Point.hpp"
#include "observer/Rect.hpp"

namespace Observer {
    template <>
    Rect::Rect(const cv::Rect& t)
        : x(t.x), y(t.y), width(t.width), height(t.height) {}

    template <>
    Rect::operator cv::Rect() {
        return cv::Rect(this->x, this->y, this->width, this->height);
    }

    template <>
    Point::Point(const cv::Point& t) : x(t.x), y(t.y) {}

    template <>
    Point::operator cv::Point() {
        return cv::Point(this->x, this->y);
    }
}  // namespace Observer