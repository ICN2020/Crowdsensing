/**
 * @brief
 * @author Yuki Koizumi
 */
#ifndef SESSION_MANAGER_HPP_INC
#define SESSION_MANAGER_HPP_INC

#include <unordered_map>
#include <string>
#include <cstdint>
#include <memory>
#include <thread>
#include <mutex>
#include <utility>
#include <ostream>
#include <boost/asio/steady_timer.hpp>

namespace ndn {
class Name;
class Data;
};
class Session;

class SessionManager
{
 public:
  using key_type     = uint64_t;
  using session_type = Session;
  using session_ptr  = std::shared_ptr<Session>;
  using value_type   = std::pair<key_type, session_type>;
  using mutex_type   = std::recursive_mutex;

 private:
  SessionManager() = default;
  ~SessionManager() noexcept = default;

  SessionManager(const SessionManager &other) = delete;
  SessionManager& operator=(const SessionManager &other) = delete;
  SessionManager(SessionManager &&other) = delete;
  SessionManager& operator=(SessionManager &&other) = delete;

 public:
  static SessionManager &instance();

  void add(key_type key, std::shared_ptr<ndn::Data> data_packet,
           std::shared_ptr<boost::asio::steady_timer> timer);
  void erase(key_type key);

  bool append_payload(key_type key, const uint8_t *value, size_t length);
  const std::vector<uint8_t> &buffer(key_type key);

  std::shared_ptr<ndn::Data> data_packet(key_type key);

  void dump(std::ostream &os) const;

 private:
  session_ptr get(key_type key);

 private:
  std::unordered_map<key_type, session_ptr> m_hash_table;
  // std::mutex m_mutex;
  std::recursive_mutex m_mutex;
};


#endif
