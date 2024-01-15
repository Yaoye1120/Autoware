#include "steering_wheel_display.hpp"

#include <QFontDatabase>
#include <QPainter>
#include <rviz_rendering/render_system.hpp>

#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <memory>
#include <string>

namespace awf_2d_overlay_vehicle
{

SteeringWheelDisplay::SteeringWheelDisplay()
{
  int fontId =
    QFontDatabase::addApplicationFont(":/assets/font/Quicksand/static/Quicksand-Regular.ttf");
  int fontId2 =
    QFontDatabase::addApplicationFont(":/assets/font/Quicksand/static/Quicksand-Bold.ttf");
  if (fontId == -1 || fontId2 == -1) {
    std::cout << "Failed to load the Quicksand font.";
  }

  // Load the wheel image
  wheelImage.load(":/assets/images/wheel.png");
  scaledWheelImage = wheelImage.scaled(54, 54, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void SteeringWheelDisplay::updateSteeringData(
  const autoware_auto_vehicle_msgs::msg::SteeringReport::ConstSharedPtr & msg)
{
  try {
    // Assuming msg->steering_angle is the field you're interested in
    float steeringAngle = msg->steering_tire_angle;
    // we received it as a radian value, but we want to display it in degrees
    steering_angle_ =
      (steeringAngle * -180 / M_PI) *
      17;  // 17 is the ratio between the steering wheel and the steering tire angle i assume
  } catch (const std::exception & e) {
    // Log the error
    std::cerr << "Error in processMessage: " << e.what() << std::endl;
  }
}

void SteeringWheelDisplay::drawSteeringWheel(QPainter & painter, const QRectF & backgroundRect)
{
  // Enable Antialiasing for smoother drawing
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  QImage wheel = coloredImage(scaledWheelImage, gray);

  // Rotate the wheel
  qreal steeringAngle = steering_angle_;  // No need to round here
  // Calculate the position
  int wheelCenterX = backgroundRect.right() - wheel.width() - 17.5;
  int wheelCenterY = backgroundRect.height() - wheel.height() + 15;

  // Rotate the wheel image
  QTransform rotationTransform;
  rotationTransform.translate(wheel.width() / 2.0, wheel.height() / 2.0);
  rotationTransform.rotate(steeringAngle);
  rotationTransform.translate(-wheel.width() / 2.0, -wheel.height() / 2.0);

  QImage rotatedWheel = wheel.transformed(rotationTransform, Qt::SmoothTransformation);

  QPointF drawPoint(
    wheelCenterX - rotatedWheel.width() / 2, wheelCenterY - rotatedWheel.height() / 2);

  // Draw the rotated image
  painter.drawImage(drawPoint.x(), drawPoint.y(), rotatedWheel);

  QString steeringAngleStringAfterModulo = QString::number(fmod(steeringAngle, 360), 'f', 0);

  // Draw the steering angle text
  QFont steeringFont("Quicksand", 9, QFont::Bold);
  painter.setFont(steeringFont);
  painter.setPen(QColor(0, 0, 0, 255));
  QRect steeringRect(
    wheelCenterX - wheelImage.width() / 2, wheelCenterY - wheelImage.height() / 2,
    wheelImage.width(), wheelImage.height());
  painter.drawText(steeringRect, Qt::AlignCenter, steeringAngleStringAfterModulo + "°");
}

QImage SteeringWheelDisplay::coloredImage(const QImage & source, const QColor & color)
{
  QImage result = source;
  QPainter p(&result);
  p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
  p.fillRect(result.rect(), color);
  p.end();
  return result;
}

}  // namespace awf_2d_overlay_vehicle