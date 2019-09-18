/**
 * @brief
 * @author Yuki Koizumi
 */
#include "session-manager.hpp"

#include <ndn-cxx/name.hpp>
#include <ndn-cxx/data.hpp>

#include <fcopss/log.hpp>
#include <cinttypes>
#include <vector>
#include <ostream>

#include <boost/asio.hpp>

class Session
{
 public:
  Session() = delete;
  Session(uint64_t id_, std::shared_ptr<ndn::Data> data_,
          std::shared_ptr<boost::asio::steady_timer> timer_)
      : session_id(id_),
        data_ptr(data_),
        timer_ptr(timer_)
  {}
  ~Session() noexcept = default;

  Session(const Session &other) = default;
  Session& operator=(const Session &other) = default;
  Session(Session &&other) noexcept = default;
  Session& operator=(Session &&other) noexcept = default;

  // void append_payload(const std::string &str);

 public:
  const uint64_t             session_id;
  std::shared_ptr<ndn::Data> data_ptr;
  std::shared_ptr<boost::asio::steady_timer> timer_ptr;
  std::vector<uint8_t>       buffer;
};

// void Session::append_payload(const std::string &str)
// {
//   buffer.append(str);
// }

SessionManager &SessionManager::instance()
{
  static SessionManager object;
  return object;
}

void SessionManager::add(key_type key, std::shared_ptr<ndn::Data> data_packet,
                         std::shared_ptr<boost::asio::steady_timer> timer)
{
  std::lock_guard<mutex_type> lock(m_mutex);
  if(m_hash_table.find(key) == m_hash_table.end()) {
    m_hash_table.emplace(key, session_ptr(new session_type(key, data_packet, timer)));
  }
  return;
}

SessionManager::session_ptr SessionManager::get(key_type key)
{
  std::lock_guard<mutex_type> lock(m_mutex);
  try {
    return m_hash_table.at(key);
  }
  catch(std::out_of_range&) {
    return session_ptr();
  }
}

void SessionManager::erase(key_type key)
{
  std::lock_guard<mutex_type> lock(m_mutex);
  m_hash_table.erase(key);
}

bool SessionManager::append_payload(key_type key, const uint8_t *value, size_t length)
{
  std::lock_guard<mutex_type> lock(m_mutex);
  session_ptr data = get(key);
  if(data == nullptr) {
    INFO("Session (%PRIu64) does not exist.", key);
    return false;
  }
  std::copy(value, value + length, std::back_inserter(data->buffer));
  data->buffer.push_back('\n');
  return true;
}

const std::vector<uint8_t> &SessionManager::buffer(key_type key)
{
  std::lock_guard<mutex_type> lock(m_mutex);
  return m_hash_table.at(key)->buffer;
}

std::shared_ptr<ndn::Data> SessionManager::data_packet(key_type key)
{
  std::lock_guard<mutex_type> lock(m_mutex);
  session_ptr data = get(key);
  if(data == nullptr) {
    INFO("Session (%PRIu64) does not exist.", key);
    return std::shared_ptr<ndn::Data>();
  }
  return data->data_ptr;
}

void SessionManager::dump(std::ostream &os) const {
  std::for_each(m_hash_table.begin(), m_hash_table.end(),
                [&os](const std::pair<key_type, session_ptr> &p) {
                  os << '{' << p.first << ',' << p.second->data_ptr->getName().toUri().c_str() << "}, " << std::endl;
                });
  return;
}
