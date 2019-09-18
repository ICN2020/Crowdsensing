#ifndef PRODUCER_HPP_INC
#define PRODUCER_HPP_INC

#include <memory>
#include <random>
#include <thread>
#include <vector>
#include <unordered_map>

#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

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
};

class Producer : boost::noncopyable {
 public:
  Producer(int mode, detector_ptr detector);
  ~Producer();
  void run();
  void adddata(std::string result);

  ndn::KeyChain m_key_chain;

  ndn::Face            m_ndn_face;
  
  std::unique_ptr<ndn::Face>  m_ndn_face_ptr;

  const uint_fast32_t m_num_thread;
  const uint_fast32_t m_num_queue;
  uint_fast32_t       m_current_queue;
  

  std::vector<boost::asio::io_service>       m_io_service_pool;
  std::vector<boost::asio::io_service::work> m_worker_pool;
  std::vector<std::thread>                   m_thread_pool;

  boost::asio::io_service m_timer_service;
  std::shared_ptr<boost::asio::io_service::work> m_timer_worker;

  static const uint_fast32_t m_timeout_second = 10000;

  std::mt19937_64 m_id_generator;

  uint8_t edge_mode;
  detector_ptr m_detector;

  std::vector<std::string> target_name;
  std::vector<std::string> loc_name;

  uint64_t ArribalInterests;
  uint64_t AllInterests;

 private:
  void onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest);
  void onInterest_Cloud(const ndn::InterestFilter& filter, const ndn::Interest& interest);
  void onRegisterFailed(const ndn::Name& prefix, const std::string& reason);
  void onData(const ndn::Interest&, const ndn::Data& data);
  void onNack(const ndn::Interest&, ndn::lp::Nack& nack);
  void onTimeout(const ndn::Interest& interest);
  
  void send_data(const boost::system::error_code& error, uint64_t session_id);
  //この3つはまとめる
  std::vector<std::string> convertInterestName(const std::string &interest_name);
  std::vector<std::string> ExtractTargets(const std::string &interest_name);
  std::vector<std::string> ExtractLocname(const std::string &interest_name);
  std::string decodeURI(const std::string &uri);

  template <class F>
  void post_task(F f);
  boost::asio::io_service& io_service();

};
#endif
