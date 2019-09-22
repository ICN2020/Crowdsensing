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
#ifndef EXECUTE_HPP_INC
#define EXECUTE_HPP_INC

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest-filter.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/time.hpp>

#include <opencv2/opencv.hpp>
#include "objectdetection.hpp"
#include "producer.hpp"

namespace ndn {
class Interest;
class Data;
class InterestFilter;
class Name;
};  // namespace ndn

class Execute {
 public:
  Execute(std::string prefix, std::string loc, std::string target, std::string sid, detector_ptr detector, Producer* prod);
  ~Execute();

  std::string fetcher_name;
  std::string location_name;
  std::string target_name;
  std::string session_id;
  detector_ptr m_detector;
  Producer* producer;

 public:
  void afterFetchComplete(const ndn::ConstBufferPtr& data);
  void afterFetchError(uint32_t errorCode, const std::string& ErrorMsg);
};

#endif
