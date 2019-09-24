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
#ifndef PRODUCER_HPP_INC
#define PRODUCER_HPP_INC

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
#include <ndn-cxx/security/validator-null.hpp>

#include "objectdetection.hpp"
//#include "execute.hpp"

namespace ndn {
class Interest;
class Data;
class InterestFilter;
class Name;
};  // namespace ndn

class Producer : boost::noncopyable {
 public:
  Producer(int mode, detector_ptr detector);
  ~Producer();
  void run();
  void adddata(std::string result);

 private:
  void onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest);
  void onInterest_Cloud(const ndn::InterestFilter& filter, const ndn::Interest& interest);
  void onRegisterFailed(const ndn::Name& prefix, const std::string& reason);
  void onData(const ndn::Interest&, const ndn::Data& data);
  void onNack(const ndn::Interest&, ndn::lp::Nack& nack);
  void onTimeout(const ndn::Interest& interest);

  void send_data(const boost::system::error_code& error, uint64_t session_id);
  //この3つはまとめる
  void convertInterestName(const std::string& interest_name, std::vector<std::string> &interest_name_list);
  std::vector<std::string> ExtractTargets(const std::string& interest_name);
  std::vector<std::string> ExtractLocname(const std::string& interest_name);
  std::string decodeURI(const std::string& uri);

  template <class F> void post_task(F f);
  boost::asio::io_service& io_service();

 private:
  ndn::KeyChain m_key_chain;
  ndn::Face     m_ndn_face;

  std::unique_ptr<ndn::Face> m_ndn_face_ptr;

  const uint_fast32_t m_num_thread;
  const uint_fast32_t m_num_queue;
  uint_fast32_t m_current_queue;

  std::vector<boost::asio::io_service> m_io_service_pool;
  std::vector<boost::asio::io_service::work> m_worker_pool;
  std::vector<std::thread> m_thread_pool;

  boost::asio::io_service m_timer_service;
  std::shared_ptr<boost::asio::io_service::work> m_timer_worker;

  static const uint_fast32_t m_timeout_second = 10000;

  std::mt19937_64 m_id_generator;

  const uint8_t m_edge_mode;
  detector_ptr m_detector;

  std::vector<std::string> m_target_name;
  std::vector<std::string> m_location_name;

  uint64_t m_number_of_arrival_interest;
  uint64_t m_issued_interest;

};
#endif
