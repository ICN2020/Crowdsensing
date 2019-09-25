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
#ifndef EXECUTOR_HPP_INC
#define EXECUTOR_HPP_INC

#include "objectdetection.hpp"
#include "producer.hpp"

#include <ndn-cxx/encoding/buffer.hpp>

class Executor {
 public:
  Executor(std::string prefix, std::string location, std::string target,
           std::string session_id, detector_ptr detector, Producer* producer);
  ~Executor();

  void afterFetchComplete(const ndn::ConstBufferPtr& data);
  void afterFetchError(uint32_t errorCode, const std::string& ErrorMsg);

 private:
  const std::string  m_fetcher_name;
  const std::string  m_location_name;
  const std::string  m_target_name;
  const std::string  m_session_id;
  detector_ptr m_detector;
  Producer*    m_producer;
};

#endif
