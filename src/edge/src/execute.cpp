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
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "execute.hpp"
#include "encode.hpp"

#include <ndn-cxx/encoding/tlv.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/buffer.hpp>

Executor::Executor(std::string prefix, std::string location, std::string target,
                   std::string session_id, detector_ptr detector, Producer *producer)
    : m_fetcher_name(prefix),
      m_location_name(location),
      m_target_name(target),
      m_session_id(session_id),
      m_detector(detector),
      m_producer(producer)
{
}

Executor::~Executor() {}

void Executor::afterFetchComplete(const ndn::ConstBufferPtr& data)
{
  // object detection process and the same process as onData
  std::cerr << "got data " << std::endl;

  const ndn::Block wire(ndn::tlv::AppPrivateBlock1, data);
  // const ndn::Block wire(UINT8_WIDTH, data);
  const size_t length = wire.value_size();
  const uint8_t* value_ptr = wire.value();
  const std::vector<uint8_t> value(&value_ptr[0], &value_ptr[length]);
  cv::Mat raw(value);
  raw = raw.reshape(3, 480);  // should be change rows automatically

  std::vector<std::string> detection_result;
  m_detector->detect(raw, detection_result);

  std::cerr << "Specified target: [" << m_target_name << "]" << std::endl;
  std::cerr << "List of detected objects: [" << std::endl;
  for(const std::string& line : detection_result) {
    std::cerr << line << std::endl;
  }
  std::cerr << "]" << std::endl;

  bool is_found = false;
  if(std::find(detection_result.begin(), detection_result.end(), m_target_name) != detection_result.end()) {
    std::cout << "------------------------------------" << std::endl;
    std::cout << "----------[Target Found!]-----------" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    is_found = true;
  } else {
    std::cout << "------------------------------------" << std::endl;
    std::cout << "--------[Target Not Found!]---------" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    is_found = false;
  }
  std::string result_str = Encoder::encode(m_location_name, "", m_target_name, is_found, m_session_id);
  m_producer->adddata(result_str);
}

void Executor::afterFetchError(uint32_t errorCode, const std::string& ErrorMsg)
{
  std::cerr << errorCode << " " << ErrorMsg << std::endl;
}
