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
 */
#include "decode.hpp"
#include <iostream>
#include <json11.hpp>

Decoder::Decoder(const std::string &str)
{
  std::string error;
  m_json_object = json11::Json::parse(str, error);
  if(!error.empty()) {
    std::cerr << "json parse error: " << str << std::endl;
  }
}

std::string Decoder::target_type() const
{
  if(m_json_object.is_null()) return std::string();

  const json11::Json &obj = m_json_object["target"];
  if(obj.is_null()) {
    return std::string();
  }

  return obj.string_value();
}

std::string Decoder::session_id() const
{
  if(m_json_object.is_null()) return std::string();

  const json11::Json &obj = m_json_object["session_id"];
  if(obj.is_null()) {
    return std::string();
  }

  return obj.string_value();
}
