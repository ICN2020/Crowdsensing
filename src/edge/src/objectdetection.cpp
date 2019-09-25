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
 *
 * @author Yoji Yamamoto
 *
 */
#include "objectdetection.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

EmulateObjectDetection::EmulateObjectDetection() : ObjectDetection(), m_classes(0)
{
  const std::string classesFile = "./config/coco.names";
  std::ifstream ifs(classesFile.c_str());

  std::string line;
  while(std::getline(ifs, line)) {
    if(line.compare("person") != 0) m_classes.push_back(line);
  }
}

void EmulateObjectDetection::detect(cv::Mat frame, std::vector<std::string>& result)
{
  std::random_device rand_dev;
  std::mt19937 engine(rand_dev());

  std::shuffle(m_classes.begin(), m_classes.end(), engine);
  std::copy_n(m_classes.begin(), 10, std::back_inserter(result));

  return;
}

DnnObjectDetection::DnnObjectDetection(const std::string& dummy_file)
    : ObjectDetection(),
      m_dummy_file(dummy_file),
      m_is_dummy_mode(!dummy_file.empty()),
      m_is_verbose(false)
{
  // Load names of classes
  const std::string classesFile = "./config/coco.names";
  // Give the configuration and weight files for the model
  const cv::String modelConfiguration = "./config/yolov3.cfg";
  const cv::String modelWeights = "./config/yolov3.weights";

  std::ifstream ifs(classesFile.c_str());

  std::string line;
  while(std::getline(ifs, line)) m_classes.push_back(line);

  // Load the network
  m_net = cv::dnn::readNetFromDarknet(modelConfiguration, modelWeights);
  m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

  /* if(m_is_dummy_mode == false) {  // ダミーを使う
    int device_id = 0;
    int api_id = cv::CAP_ANY;
    m_camera.open(device_id, api_id);
    if(!m_camera.isOpened()) {
      std::cerr << "ERROR! Unable to open camera" << std::endl;
      throw CameraOpenException();
    }
  }

  m_run = true;
  m_thread = std::thread([this]() { this->capture(); }); */
}
DnnObjectDetection::~DnnObjectDetection()
{
  if(m_run == true) {
    m_run = false;
    m_thread.join();
  }
}

void DnnObjectDetection::capture()
{
  while(m_run) {
    if(m_is_dummy_mode == true) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_frame = cv::imread(m_dummy_file);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    } else {
      if(m_camera.grab()) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_camera.retrieve(m_frame);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  return;
}

void DnnObjectDetection::detect(cv::Mat frame, std::vector<std::string>& result)
{
  std::string str;
  cv::Mat blob;
  std::cerr << " in func " << std::endl;

  /* {
    std::lock_guard<std::mutex> lock(m_mutex);
    frame = m_frame.clone();
  } */

  // Create a 4D blob from a frame.
  cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cvSize(m_input_width, m_input_height), cv::Scalar(0, 0, 0),
                         true, false);

  std::cerr << " in blob " << std::endl;

  // Sets the input to the network
  m_net.setInput(blob);

  // Runs the forward pass to get output of the output layers
  std::vector<cv::Mat> outs;
  m_net.forward(outs, getOutputsNames());

  // Remove the bounding boxes with low confidence
  postprocess(frame, outs, result);

  if(m_is_verbose) {
    // Put efficiency information. The function getPerfProfile returns the overall time for
    // inference(t) and the timings for each of the layers(in layersTimes)
    std::vector<double> layersTimes;
    double freq = cv::getTickFrequency() / 1000;
    double t = m_net.getPerfProfile(layersTimes) / freq;
    std::string label = cv::format("Inference time for a frame : %.2f ms", t);
    cv::putText(frame, label, cv::Point(0, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                cv::Scalar(0, 0, 255));

    // Write the frame with the detection boxes
    cv::Mat detectedFrame;
    frame.convertTo(detectedFrame, CV_8U);
    cv::imwrite("camera.jpg", detectedFrame);
  }

  return;
}

// Remove the bounding boxes with low confidence using non-maxima suppression
void DnnObjectDetection::postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs,
                                     std::vector<std::string>& result)
{
  std::vector<int> classIds;
  std::vector<float> confidences;
  std::vector<cv::Rect> boxes;

  for(size_t i = 0; i < outs.size(); ++i) {
    // Scan through all the bounding boxes output from the network and keep only the
    // ones with high confidence scores. Assign the box's class label as the class
    // with the highest score for the box.
    float* data = (float*) outs[i].data;
    for(int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
      cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
      cv::Point classIdPoint;
      double confidence;
      // Get the value and location of the maximum score
      minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
      if(confidence > m_conf_threshold) {
        int centerX = (int) (data[0] * frame.cols);
        int centerY = (int) (data[1] * frame.rows);
        int width = (int) (data[2] * frame.cols);
        int height = (int) (data[3] * frame.rows);
        int left = centerX - width / 2;
        int top = centerY - height / 2;

        classIds.push_back(classIdPoint.x);
        confidences.push_back((float) confidence);
        boxes.push_back(cv::Rect(left, top, width, height));
      }
    }
  }

  // Perform non maximum suppression to eliminate redundant overlapping boxes with
  // lower confidences
  std::vector<int> indices;
  cv::dnn::NMSBoxes(boxes, confidences, m_conf_threshold, m_nms_threshold, indices);
  for(size_t i = 0; i < indices.size(); ++i) {
    int idx = indices[i];
    cv::Rect box = boxes[idx];
    result.push_back(m_classes[classIds[idx]]);
    drawPred(classIds[idx], confidences[idx], box.x, box.y, box.x + box.width, box.y + box.height,
             frame);
  }
}

// Draw the predicted bounding box
void DnnObjectDetection::drawPred(int classId, float conf, int left, int top, int right, int bottom,
                                  cv::Mat& frame)
{
  // Draw a rectangle displaying the bounding box
  rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(255, 178, 50), 3);

  // Get the label for the class name and its confidence
  std::string label = cv::format("%.2f", conf);
  if(!m_classes.empty()) {
    CV_Assert(classId < static_cast<int>(m_classes.size()));
    label = m_classes[classId] + ":" + label;
  }

  std::cout << m_classes[classId] << "," << top << "," << left << "," << right << "," << bottom
            << std::endl;

  // Display the label at the top of the bounding box
  int baseLine;
  cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
  top = std::max(top, labelSize.height);
  rectangle(frame, cv::Point(left, top - round(1.5 * labelSize.height)),
            cv::Point(left + round(1.5 * labelSize.width), top + baseLine),
            cv::Scalar(255, 255, 255), cv::FILLED);
  putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0),
          1);
}

// Get the names of the output layers
std::vector<cv::String> DnnObjectDetection::getOutputsNames()
{
  static std::vector<cv::String> names;
  if(names.empty()) {
    // Get the indices of the output layers, i.e. the layers with unconnected outputs
    std::vector<int> outLayers = m_net.getUnconnectedOutLayers();

    // get the names of all the layers in the network
    std::vector<cv::String> layersNames = m_net.getLayerNames();

    // Get the names of the output layers in names
    names.resize(outLayers.size());
    for(size_t i = 0; i < outLayers.size(); ++i) names[i] = layersNames[outLayers[i] - 1];
  }
  return names;
}
