#include <exception>
#include <iostream>
#include "producer.hpp"
#include "objectdetection.hpp"

int main(int argc, char** argv) {
  if (argc < 2){
    fprintf(stderr, "Usage: %s option[-c | -e]\n",argv[0]);
    return 1;
  }

  int c = getopt(argc,argv,"ce");
  if ((c != 'c') && (c != 'e')){
    fprintf(stderr, "Usage: %s option[-c | -e]\n",argv[0]);
    return 2;
  }
  
  try {
    detector_ptr detector;
    std::string dummy_file;
    detector = detector_ptr(new DnnObjectDetection(dummy_file));
    Producer producer(c,detector);
    producer.run();
  } catch(const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
