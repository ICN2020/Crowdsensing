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
#include <unistd.h>
#include <functional>
#include <iostream>

#include "objectdetection.hpp"
#include "subscribe.hpp"
#include "worker.hpp"

int main(int argc, char **argv)
{
  if(argc < 3) {
    fprintf(stderr, "Usage: %s -c CD\n", argv[0]);
    return 1;
  }

  int c;
  char *cd = nullptr;
  std::string dummy_file;
  bool emulation_mode = false;

  while((c = getopt(argc, argv, "c:d:e")) != -1) {
    switch(c) {
      case 'c':
        cd = optarg;
        break;
      case 'd':
        dummy_file = optarg;
        break;
      case 'e':
        emulation_mode = true;
        break;
      default:
        fprintf(stderr, "Usage: %s -c CD\n", argv[0]);
        return 2;
    }
  }

  if(cd == nullptr) {
    fprintf(stderr, "Usage: %s -c CD\n", argv[0]);
    return 1;
  }

  try {
    detector_ptr detector;
    if(emulation_mode) {
      detector = detector_ptr(new EmulateObjectDetection());
    } else {
      detector = detector_ptr(new DnnObjectDetection(dummy_file));
    }
    //fcopss::client::Subscriber subscriber(cd, detector);
    Worker worker(cd,detector);
    worker.run();
  } catch(std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
