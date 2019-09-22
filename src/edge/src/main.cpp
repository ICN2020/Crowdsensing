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
#include <exception>
#include <iostream>
#include "objectdetection.hpp"
#include "producer.hpp"

int main(int argc, char** argv)
{
  if(argc < 2) {
    fprintf(stderr, "Usage: %s option[-c | -e]\n", argv[0]);
    return 1;
  }

  int c = getopt(argc, argv, "ce");
  if((c != 'c') && (c != 'e')) {
    fprintf(stderr, "Usage: %s option[-c | -e]\n", argv[0]);
    return 2;
  }

  try {
    detector_ptr detector;
    std::string dummy_file;
    detector = detector_ptr(new DnnObjectDetection(dummy_file));
    Producer producer(c, detector);
    producer.run();
  } catch(const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
