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
#include <functional>
#include <iostream>

#include "objectdetection.hpp"
#include "worker.hpp"
#include "parameter.hpp"

int main(int argc, char **argv)
{
  Parameter::instance().parse(argc, argv);

  try {
    detector_ptr detector;
    if(Parameter::instance().is_emulation_mode()) {
      detector = detector_ptr(new EmulateObjectDetection());
    } else {
      detector = detector_ptr(new DnnObjectDetection(Parameter::instance().dummy_file()));
    }
    Worker worker(Parameter::instance().cd(), detector);
    worker.run();
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
