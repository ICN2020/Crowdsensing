#ifndef EXECUTE_HPP_INC
#define EXECUTE_HPP_INC


#include <ndn-cxx/face.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/interest-filter.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/time.hpp>

#include <opencv2/opencv.hpp>
#include "objectdetection.hpp"
#include "producer.hpp"

namespace ndn {
class Interest;
class Data;
class InterestFilter;
class Name;
};

class Execute{
public:
  Execute(std::string prefix, std::string loc,  std::string target, std::string sid, detector_ptr detector, Producer* prod);
  ~Execute();

  std::string fetcher_name;
  std::string location_name;
  std::string target_name;
  std::string session_id;
  detector_ptr m_detector;
  Producer* producer;

public:
  void afterFetchComplete(const ndn::ConstBufferPtr& data);
  void afterFetchError(uint32_t errorCode, const std::string& ErrorMsg);
};

#endif