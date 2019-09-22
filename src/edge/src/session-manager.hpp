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
#ifndef SESSION_MANAGER_HPP_INC
#define SESSION_MANAGER_HPP_INC

#include <boost/asio/steady_timer.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

namespace ndn {
class Name;
class Data;
};  // namespace ndn
class Session;

class SessionManager {
 public:
  using key_type = uint64_t;
  using session_type = Session;
  using session_ptr = std::shared_ptr<Session>;
  using value_type = std::pair<key_type, session_type>;
  using mutex_type = std::recursive_mutex;

 private:
  SessionManager() = default;
  ~SessionManager() noexcept = default;

  SessionManager(const SessionManager &other) = delete;
  SessionManager &operator=(const SessionManager &other) = delete;
  SessionManager(SessionManager &&other) = delete;
  SessionManager &operator=(SessionManager &&other) = delete;

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
