/**
 * Copyright (c) 2019 Osaka University
 *
 * This software is released under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef OBJECTDETECTION_HPP_INC
#define OBJECTDETECTION_HPP_INC

#include <exception>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <opencv2/core/types_c.h>
#include <opencv2/dnn.hpp>
#include <opencv2/videoio.hpp>

using detector_ptr = std::shared_ptr<class ObjectDetection>;

class ObjectDetection {
 public:
  ObjectDetection() = default;
  virtual ~ObjectDetection() = default;

  // ムーブはOK
  ObjectDetection(ObjectDetection&&) = default;
  ObjectDetection& operator=(ObjectDetection&&) = default;

  // コピー禁止
  ObjectDetection(const ObjectDetection&) = delete;
  ObjectDetection& operator=(const ObjectDetection&) = delete;

  virtual void detect(cv::Mat frame,std::vector<std::string>& result) = 0;
};

class EmulateObjectDetection : public ObjectDetection {
 public:
  EmulateObjectDetection();
  ~EmulateObjectDetection() {}
  void detect(cv::Mat frame,std::vector<std::string>& result) override;

 private:
  std::vector<std::string> m_classes;
};

class DnnObjectDetection : public ObjectDetection {
 public:
  DnnObjectDetection(const std::string& dummy_file);
  ~DnnObjectDetection();
  void detect(cv::Mat frame, std::vector<std::string>& result) override;

  // ムーブはOK
  DnnObjectDetection(DnnObjectDetection&&) = default;
  DnnObjectDetection& operator=(DnnObjectDetection&&) = default;

  // コピー禁止
  DnnObjectDetection(const DnnObjectDetection&) = delete;
  DnnObjectDetection& operator=(const DnnObjectDetection&) = delete;

 private:
  void postprocess(cv::Mat& frame, const std::vector<cv::Mat>& out,
                   std::vector<std::string>& result);
  void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);
  std::vector<cv::String> getOutputsNames();
  void capture();

 private:
  const float confThreshold = 0.5;  // Confidence threshold
  const float nmsThreshold = 0.4;   // Non-maximum suppression threshold
  const int inpWidth = 416;         // Width of network's input image
  const int inpHeight = 416;        // Height of network's input image
  const std::string m_dummy_file;
  const bool m_is_dummy_mode;
  const bool m_is_verbose;

  cv::VideoCapture m_camera;
  cv::Mat m_frame;
  cv::dnn::Net m_net;
  std::vector<std::string> m_classes;
  std::thread m_thread;
  mutable std::mutex m_mutex;

  bool m_run;
};

class CameraOpenException : public std::exception {
 public:
  CameraOpenException() : std::exception() {}
  virtual const char* what() const noexcept { return "Cannot open camera devices."; }
};

#endif
