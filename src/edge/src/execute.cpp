#include "execute.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <chrono>

#include "encode.hpp"


Execute::Execute(std::string prefix, std::string loc, std::string target, std::string sid, detector_ptr detector,Producer* prod) 
    : fetcher_name(prefix),
      location_name(loc),
      target_name(target),
      session_id(sid),
      m_detector(detector),
      producer(prod)
      {}

Execute::~Execute(){}

void Execute::afterFetchComplete(const ndn::ConstBufferPtr& data){
    // object detection process and the same process as onData
  std::cerr << "got data " << std::endl;

  //const ndn::Block wire(&tmp,tmp.size());
  //const ndn::Block wire(data,data->begin(),data->end(),true);
  const ndn::Block wire(UINT8_WIDTH,data);
  const size_t length = wire.value_size();
  const uint8_t* value_ptr  = wire.value(); 
  const std::vector<uint8_t> value(&value_ptr[0],&value_ptr[length]);
  cv::Mat raw(value);
  raw = raw.reshape(3,480); //should be change rows automatically

   std::vector<std::string> detection_result;
    m_detector->detect(raw,detection_result);

    std::cerr << "Specified target: [" << target_name << "]" << std::endl;
    std::cerr << "List of detected objects: [" << std::endl;
    for(const std::string& line : detection_result) {
      std::cerr << line << std::endl;
    }
    std::cerr << "]" << std::endl;

    bool is_found = false;
    if(std::find(detection_result.begin(), detection_result.end(), target_name) !=
       detection_result.end()) {
      std::cout << "------------------------------------" << std::endl;
      std::cout << "----------[Target Found!]-----------" << std::endl;
      std::cout << "------------------------------------" << std::endl;
      is_found = true;
    } else {
      std::cout << "------------------------------------" << std::endl;
      std::cout << "--------[Target Not Found!]---------" << std::endl;
      std::cout << "------------------------------------" << std::endl;
      is_found = false;
    }
    std::string result_str = Encoder::encode(location_name,"",target_name,is_found,session_id);
    producer->adddata(result_str);
}


void Execute::afterFetchError(uint32_t errorCode, const std::string& ErrorMsg){
     std::cerr << errorCode << " " << ErrorMsg << std::endl;
}
