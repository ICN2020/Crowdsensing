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
#ifndef WORKER_HPP_INC
#define WORKER_HPP_INC

#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/time.hpp>

#include "objectdetection.hpp"

namespace ndn {
class Interest;
class Data;
class InterestFilter;
class Name;
};  // namespace ndn

class Worker;

class Worker : boost::noncopyable {
 public:
  Worker(const std::string& cd_str, detector_ptr detector);
  ~Worker();
  void run();

  const std::string m_cd_string;
  detector_ptr m_detector;

  struct Options {
    ndn::security::SigningInfo signingInfo;
    // ndn::time freshnessPeriod = 10000;
    size_t maxSegmentSize = 8000;
    bool isQuiet = false;
    bool isVerbose = false;
    bool wantShowVersion = false;
  };

 public:
  ndn::KeyChain m_key_chain;
  ndn::Face m_ndn_face;

  std::unique_ptr<ndn::Face> m_ndn_face_ptr;

  const uint_fast32_t m_num_thread;
  const uint_fast32_t m_num_queue;
  uint_fast32_t m_current_queue;

  std::vector<boost::asio::io_service> m_io_service_pool;
  std::vector<boost::asio::io_service::work> m_worker_pool;
  std::vector<std::thread> m_thread_pool;

  boost::asio::io_service m_timer_service;
  std::shared_ptr<boost::asio::io_service::work> m_timer_worker;

  static const uint_fast32_t m_timeout_second = 1500;

  std::mt19937_64 m_id_generator;

  ndn::Name m_prefix;
  ndn::Name m_versionedPrefix;
  const Options m_options;

  NDN_CXX_PUBLIC_WITH_TESTS_ELSE_PRIVATE : std::vector<std::shared_ptr<ndn::Data>> m_store;

 private:
  void onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest);
  void onRegisterFailed(const ndn::Name& prefix, const std::string& reason);
  std::string ExtractSessionID(const std::string& interest_name);
  std::vector<std::string> ExtractTargets(const std::string& interest_name);
  std::string decodeURI(const std::string& uri);
  void processSegmentInterest(const ndn::Interest& interest);
  // void populateStore(std::istream& is);
  void populateStore(std::vector<uint8_t> data_vector);
};
#endif
