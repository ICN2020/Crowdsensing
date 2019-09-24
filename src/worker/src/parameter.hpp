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
 * @author Yuki Koizumi
 */
#ifndef PARAMETER_HPP_INC
#define PARAMETER_HPP_INC

#include <string>

class Parameter
{
 public:
  ~Parameter() = default;
  Parameter(const Parameter &) = delete;
  Parameter &operator=(const Parameter) = delete;

  static Parameter &instance();
  void parse(int argc, char **argv);
  void print(std::ostream &os) const;
  void initialize();

  const std::string &cd() const { return m_cd; }
  const std::string &worker_name() const { return m_worker_name; }

  const std::string &location_name() const { return m_location_name; }
  const std::string &time_name() const { return m_time_name; }

  const std::string &dummy_file() const { return m_dummy_file; }
  bool is_dummy_mode() const { return m_is_dummy_mode; }
  bool is_emulation_mode() const { return m_is_emulation_mode; }

 private:
  Parameter();

 private:
  std::string m_cd;
  std::string m_worker_name;

  std::string m_location_name;
  std::string m_time_name;

  std::string m_dummy_file;
  bool        m_is_dummy_mode;
  bool        m_is_emulation_mode;
};

std::ostream &operator<<(std::ostream &os, const Parameter &obj);

#endif
