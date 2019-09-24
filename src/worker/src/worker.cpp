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
#include "worker.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <thread>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/bind.hpp>
#include <vector>

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest-filter.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/time.hpp>

#include <json11.hpp>

#include "decode.hpp"
#include "encode.hpp"

using namespace ndn::literals::time_literals;

Worker::Worker(const std::string& cd_str, detector_ptr detector)
    : m_cd_string(cd_str),
      m_detector(detector),
      m_num_thread(1),
      m_num_queue(1),
      m_current_queue(0),
      m_io_service_pool(m_num_queue),
      m_worker_pool(),
      m_thread_pool(),
      m_timer_service(),
      m_id_generator(1)
{
  for(auto&& ios : m_io_service_pool) {
    m_worker_pool.emplace_back(ios);
  }
  for(unsigned int n = 0; n < m_num_thread; ++n) {
    m_thread_pool.emplace_back([this, n] { this->m_io_service_pool.at(n % m_num_queue).run(); });
  }

  m_timer_worker.reset(new boost::asio::io_service::work(m_timer_service));
}

Worker::~Worker()
{
  m_worker_pool.clear();
  for(auto& thread : m_thread_pool) {
    if(thread.joinable()) thread.join();
  }
}

void Worker::run()
{
  std::thread timer_thread([this] { this->m_timer_service.run(); });
  std::thread ndn_thread([this] {
    // INFO("Run NFD");
    std::cerr << "Run NFD" << std::endl;
    this->m_ndn_face.setInterestFilter(m_cd_string, std::bind(&Worker::onInterest, this, _1, _2),
                                       ndn::RegisterPrefixSuccessCallback(),
                                       std::bind(&Worker::onRegisterFailed, this, _1, _2));
    this->m_ndn_face.processEvents();
  });

  timer_thread.join();
  ndn_thread.join();
  return;
}

void Worker::onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest)
{
  std::stringstream ss;
  ss << "Receive Interest packet: " << interest;
  // DEBUG("%s", ss.str().c_str());
  std::cerr << ss.str().c_str() << std::endl;
  ss.clear();
  ss.str("");

  std::string edge_mode = "c";
  if(interest.hasApplicationParameters()) {
    std::string param(interest.getApplicationParameters().value(),
                      interest.getApplicationParameters().value() + interest.getApplicationParameters().value_size());
    std::cerr << "parameter  oK: " << param << std::endl;
    edge_mode = param;
  }

  std::vector<std::string> target_name(ExtractTargets(decodeURI(interest.getName().toUri())));
  std::string session_id(ExtractSessionID(decodeURI(interest.getName().toUri())));

  if(target_name.size() == 0) {
    std::cerr << "Target is not properly specified." << std::endl;
    return;
  }
  if(edge_mode == "e") {
    std::vector<std::string> detection_result;
    m_detector->detect(detection_result);

    std::cerr << "Specified target: [" << target_name[0] << "]" << std::endl;
    std::cerr << "List of detected objects: [" << std::endl;
    for(const std::string& line : detection_result) {
      std::cerr << line << std::endl;
    }
    std::cerr << "]" << std::endl;

    bool is_found = false;
    if(std::find(detection_result.begin(), detection_result.end(), target_name[0]) !=
       detection_result.end()) {
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

    std::string m_location(m_cd_string);
    boost::algorithm::replace_all(m_location, "/", "");
    std::string result_str = Encoder::encode(m_location, "", target_name[0], is_found, session_id);
    std::vector<uint8_t> data_vector;
    std::copy(result_str.begin(), result_str.end(), std::back_inserter(data_vector));

    auto data = ndn::make_shared<ndn::Data>(interest.getName());
    data->setFreshnessPeriod(10_s);
    data->setContent(reinterpret_cast<const uint8_t*>(data_vector.data()), data_vector.size());

    m_key_chain.sign(*data);

    std::cerr << "Sending Data: " << *data << std::endl;

    m_ndn_face.put(*data);
  } else if(edge_mode == "c") {  // cloud mode--------------------------

    // if(m_store.empty()){
    if(!interest.getName()[-1].isSegment()) {
      cv::Mat raw = m_detector->returnMat();

      std::cout << raw.rows << " " << raw.cols << " " << raw.dims << " " << raw.channels()
                << std::endl;
      raw = raw.reshape(1, 1);

      std::vector<uint8_t> data_vector = raw;

      ndn::Name prefix = interest.getName();
      if(prefix.size() > 0 && prefix[-1].isVersion()) {
        m_prefix = prefix.getPrefix(-1);
        m_versionedPrefix = prefix;
      } else {
        m_prefix = prefix;
        m_versionedPrefix = ndn::Name(m_prefix).appendVersion();
      }

      m_store.clear();
      // populateStore(is);
      populateStore(data_vector);
      processSegmentInterest(interest);
    } else {
      processSegmentInterest(interest);
    }
  }
}

void Worker::onRegisterFailed(const ndn::Name& prefix, const std::string& reason)
{
  std::cerr << "ERROR: Failed to register prefix \"" << prefix << "\" in local hub's daemon ("
            << reason << ")" << std::endl;
  m_ndn_face_ptr->shutdown();
}

std::string Worker::ExtractSessionID(const std::string& interest_name)
{
  std::vector<std::string> token_list;
  boost::algorithm::split(token_list, interest_name, boost::is_any_of("/"));

  std::string sessionID(token_list.back());

  return sessionID;
}

std::vector<std::string> Worker::ExtractTargets(const std::string& interest_name)
{
  std::vector<std::string> token_list;
  boost::algorithm::split(token_list, interest_name, boost::is_any_of("/"));

  token_list.pop_back();
  std::string keyword(token_list.back());
  std::vector<std::string> key_list;
  boost::algorithm::split(key_list, keyword, boost::is_any_of(" "));

  std::string material(key_list.back());
  std::vector<std::string> material_list;
  boost::algorithm::split(material_list, material, boost::is_any_of(":"));

  std::string target(material_list.back());
  boost::algorithm::replace_all(target, "[", "");
  boost::algorithm::replace_all(target, "]", "");

  std::vector<std::string> target_list;
  boost::algorithm::split(target_list, target, boost::is_any_of(","));

  return target_list;
}

std::string Worker::decodeURI(const std::string& uri)
{
  std::string replaced_string(uri);

  boost::algorithm::replace_all(replaced_string, "%23", "#");
  boost::algorithm::replace_all(replaced_string, "%3A", ":");
  boost::algorithm::replace_all(replaced_string, "%2C", ",");
  boost::algorithm::replace_all(replaced_string, "%5B", "[");
  boost::algorithm::replace_all(replaced_string, "%5D", "]");
  boost::algorithm::replace_all(replaced_string, "%20", " ");
  boost::algorithm::replace_all(replaced_string, "%27", "'");
  boost::algorithm::replace_all(replaced_string, "%28", "(");
  boost::algorithm::replace_all(replaced_string, "%29", ")");
  boost::algorithm::replace_all(replaced_string, "%2A", "*");

  return replaced_string;
}

void Worker::populateStore(std::vector<uint8_t> data_vector)
{
  // BOOST_ASSERT(m_store.empty());
  std::cerr << "Loading input ..." << std::endl;

  // std::vector<uint8_t> buffer(m_options.maxSegmentSize);
  std::vector<uint8_t> buffer(m_options.maxSegmentSize);
  int i = 0;
  while(m_options.maxSegmentSize * i < data_vector.size()) {
    // is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    if(m_options.maxSegmentSize * (i + 1) < data_vector.size()) {
      std::copy(data_vector.begin() + m_options.maxSegmentSize * i,
                data_vector.begin() + m_options.maxSegmentSize * (i + 1), buffer.begin());
    } else {
      buffer.resize(data_vector.end() - (data_vector.begin() + m_options.maxSegmentSize * i));
      std::copy(data_vector.begin() + m_options.maxSegmentSize * i, data_vector.end(),
                buffer.begin());
    }
    const auto nCharsRead = buffer.size();
    std::cerr << nCharsRead << std::endl;

    if(nCharsRead > 0) {
      // ndn::Name interestname = interest.getName();
      auto data =
          ndn::make_shared<ndn::Data>(ndn::Name(m_versionedPrefix).appendSegment(m_store.size()));
      data->setFreshnessPeriod(10_s);
      data->setContent(buffer.data(), static_cast<size_t>(nCharsRead));
      m_store.push_back(data);
    }
    i++;
  }

  if(m_store.empty()) {
    // ndn::Name interestname = interest.getName();
    auto data = ndn::make_shared<ndn::Data>(ndn::Name(m_versionedPrefix).appendSegment(0));
    data->setFreshnessPeriod(10_s);
    m_store.push_back(data);
  }

  auto finalBlockId = ndn::Name::Component::fromSegment(m_store.size() - 1);
  for(const auto& data : m_store) {
    data->setFinalBlock(finalBlockId);
    // m_key_chain.sign(*data,m_options.signingInfo);
  }

  std::cerr << "Created " << m_store.size() << " chunks for prefix " << m_prefix << std::endl;
}

void Worker::processSegmentInterest(const ndn::Interest& interest)
{
  BOOST_ASSERT(m_store.size() > 0);

  // std::cerr << " Interest: " << interest << std::endl;

  const ndn::Name& name = interest.getName();
  // std::cout<< name[-1] << std::endl;
  ndn::shared_ptr<ndn::Data> data;

  if(name.size() == m_versionedPrefix.size() + 1 && name[-1].isSegment()) {
    const auto segmentNo = static_cast<size_t>(interest.getName()[-1].toSegment());
    // specific segment retrieval
    std::cerr << segmentNo << std::endl;
    if(segmentNo < m_store.size()) {
      data = m_store[segmentNo];
    }
  } else if(interest.matchesData(*m_store[0])) {
    std::cerr << "unspecified version or segment number, return first segment" << std::endl;
    data = m_store[0];
  }
  m_key_chain.sign(*data);
  if(data != nullptr) {
    std::cerr << "Data: " << *data << std::endl;
    m_ndn_face.put(*data);
  } else {
    std::cerr << "Interest cannot be satisfied, sending Nack" << std::endl;
    m_ndn_face.put(ndn::lp::Nack(interest));
  }
}
