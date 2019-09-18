#include "producer.hpp"
#include "session-manager.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <chrono>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/interest-filter.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/time.hpp>

#include <ndn-cxx/util/segment-fetcher.hpp>
#include <ndn-cxx/security/validator-null.hpp>

#include <json11.hpp>

#include <opencv2/opencv.hpp>

#include "execute.hpp"

using namespace ndn::literals::time_literals;

Producer::Producer(int mode, detector_ptr detector)
    : m_num_thread(0),
      m_num_queue(0),
      m_current_queue(0),
      m_io_service_pool(m_num_queue),
      m_worker_pool(),
      m_thread_pool(),
      m_timer_service(),
      m_id_generator(1),
      edge_mode(mode),
      m_detector(detector),
      ArribalInterests(0),
      AllInterests(0)
{
  for(auto &&ios: m_io_service_pool) {
    m_worker_pool.emplace_back(ios);
  }
  for(unsigned int n = 0; n < m_num_thread; ++n) {
    m_thread_pool.emplace_back([this, n] { this->m_io_service_pool.at(n % m_num_queue).run(); });
  }
  std::cerr << "mode: " << edge_mode << std::endl;

  m_timer_worker.reset(new boost::asio::io_service::work(m_timer_service));
}

Producer::~Producer() {
  m_worker_pool.clear();
  for(auto& thread : m_thread_pool) {
    if(thread.joinable()) thread.join();
  }
}

void Producer::run() {
  std::thread timer_thread([this] {
                             this->m_timer_service.run();
                           });
  if (edge_mode == 'c'){
    std::cerr << "[INFO] Cloud mode" << std::endl;
    std::thread ndn_thread([this] {
                            std::cerr << "[INFO] Run NFD" << std::endl;
                           this->m_ndn_face.setInterestFilter("/icn2020/edge",
                                                              std::bind(&Producer::onInterest_Cloud, this, _1, _2),
                                                              ndn::RegisterPrefixSuccessCallback(),
                                                              std::bind(&Producer::onRegisterFailed, this, _1, _2));
                           this->m_ndn_face.processEvents();
                         });
    ndn_thread.join();
  }else if (edge_mode == 'e'){
    std::cerr << "[INFO] Edge mode" << std::endl;
    std::thread ndn_thread([this] {
                            std::cerr << "[INFO] Run NFD" << std::endl;
                           this->m_ndn_face.setInterestFilter("/icn2020/edge",
                                                              std::bind(&Producer::onInterest, this, _1, _2),
                                                              ndn::RegisterPrefixSuccessCallback(),
                                                              std::bind(&Producer::onRegisterFailed, this, _1, _2));
                           this->m_ndn_face.processEvents();
                         });
    ndn_thread.join();
  }


  timer_thread.join();
  return;
}


void Producer::onInterest(const ndn::InterestFilter& filter, const ndn::Interest& interest) {
  std::stringstream ss;
  ss << "Receive Interest packet: " << interest;
  std::cerr << ss.str().c_str() << std::endl;
  ss.clear(); ss.str("");
  ArribalInterests = 0;
  AllInterests = 0;

  // Create new name, based on Interest's name
  ndn::Name data_name(interest.getName());
  data_name
      .append("IoT") // add "teDEBUGstAdpp" component to Interest name
      .appendVersion(); // add "version" component (current UNIX timestamp in milliseconds)

  if(!interest.hasParameters()) {
    std::cerr << "[WARN] Received packet does not have a parameter field." <<std::endl;
  }

   uint64_t session_id(m_id_generator());

  std::string interest_name(decodeURI(interest.getName().toUri()));
  std::vector<std::string> Reinvoke_name(convertInterestName(interest_name));
  AllInterests = Reinvoke_name.size();

  // Create Data packet
  std::shared_ptr<ndn::Data> data_packet(new ndn::Data());
  data_packet->setName(data_name);
  data_packet->setFreshnessPeriod(1_ms);  // 10 seconds
  m_key_chain.sign(*data_packet);


  std::shared_ptr<boost::asio::steady_timer> timer(new boost::asio::steady_timer(m_timer_service));
  timer->expires_from_now(std::chrono::milliseconds(m_timeout_second));
  timer->async_wait(boost::bind(&Producer::send_data, this, boost::asio::placeholders::error, session_id));
  SessionManager::instance().add(session_id, data_packet, timer);


  for(unsigned int j = 0;j < Reinvoke_name.size();j++ ){
  std::cerr << "[INFO] Interest name URI: "<< interest.getName().toUri().c_str() << std::endl;
  std::cerr <<"[INFO] Converted name: "<<  interest_name.c_str() << std::endl;
  std::cerr <<"[INFO] Re-invoke interest name: "<< Reinvoke_name[j].c_str() << std::endl;
    
    ndn::Name interestName(Reinvoke_name[j] + "/" + std::to_string(session_id) );
    ndn::Interest re_interest(interestName);
    re_interest.setCanBePrefix(true);
    re_interest.setMustBeFresh(true);
    re_interest.setInterestLifetime(1_s);
    std::vector<uint8_t> parameter;
    parameter.push_back(edge_mode);
    re_interest.setParameters(parameter.data(),parameter.size());
  

    std::cerr<<"Sending Interest " << re_interest << std::endl;
    
      m_ndn_face.expressInterest(re_interest,
                                      std::bind(&Producer::onData, this, _1, _2),
                                      std::bind([] {std::cerr << "Nack" << std::endl;}),
                                      std::bind([] {std::cerr << "Timeout" << std::endl;}));                              

  }
}

void Producer::onInterest_Cloud(const ndn::InterestFilter& filter, const ndn::Interest& interest) {
  std::stringstream ss;
  ss << "Receive Interest packet: " << interest;
  std::cerr << ss.str().c_str() << std::endl;
  ss.clear(); ss.str("");
  ArribalInterests = 0;
  AllInterests = 0;

  // Create new name, based on Interest's name
  ndn::Name data_name(interest.getName());
  data_name
      .append("IoT") // add "teDEBUGstAdpp" component to Interest name
      .appendVersion(); // add "version" component (current UNIX timestamp in milliseconds)

  if(!interest.hasParameters()) {
    std::cerr << "[WARN] Received packet does not have a parameter field." << std::endl;
  }

   uint64_t session_id(m_id_generator());

  target_name = ExtractTargets(decodeURI(interest.getName().toUri()));
  std::string interest_name(decodeURI(interest.getName().toUri()));
  std::vector<std::string> Reinvoke_name(convertInterestName(interest_name));
  AllInterests = Reinvoke_name.size();
  
  // Create Data packet
  std::shared_ptr<ndn::Data> data_packet(new ndn::Data());
  data_packet->setName(data_name);
  data_packet->setFreshnessPeriod(1_ms);  // 10 seconds
  m_key_chain.sign(*data_packet);

  std::shared_ptr<boost::asio::steady_timer> timer(new boost::asio::steady_timer(m_timer_service));
  timer->expires_from_now(std::chrono::milliseconds(m_timeout_second));
  timer->async_wait(boost::bind(&Producer::send_data, this, boost::asio::placeholders::error, session_id));
  SessionManager::instance().add(session_id, data_packet, timer);

  for(unsigned int j = 0;j < Reinvoke_name.size();j++ ){
  std::cerr << "[INFO] Interest name URI: "<< interest.getName().toUri().c_str() << std::endl;
  std::cerr <<"[INFO] Converted name: "<<  interest_name.c_str() << std::endl;
  std::cerr <<"[INFO] Re-invoke interest name: "<< Reinvoke_name[j].c_str() << std::endl;
    
    ndn::Name interestName(Reinvoke_name[j] + "/" + std::to_string(session_id) );
    ndn::Interest re_interest(interestName);
    re_interest.setCanBePrefix(true);
    re_interest.setMustBeFresh(true);
    loc_name = ExtractLocname(Reinvoke_name[j] + "/" + std::to_string(session_id));

    std::cerr<<"Cloud:Sending Interest " << re_interest << std::endl;

  ndn::util::SegmentFetcher::Options Opt;
    Opt.interestLifetime = 1_s;
    Opt.useConstantCwnd = true;
    Opt.initCwnd = 1;
  
  Execute executor(re_interest.getName().toUri().c_str(),loc_name[0],target_name[0],std::to_string(session_id),m_detector,this);
  std::thread ndn_thread([this,re_interest,Opt,executor] {
    auto fetcher = ndn::util::SegmentFetcher::start(m_ndn_face,re_interest,ndn::security::v2::getAcceptAllValidator(),Opt); 
    fetcher->onComplete.connect(bind(&Execute::afterFetchComplete ,executor, _1));
    fetcher->onError.connect(bind(&Execute::afterFetchError, executor, _1, _2));                                 
  });
    ndn_thread.join();
  }
  
}

void Producer::adddata(std::string result){
  const size_t length = result.length();
  const uint8_t* value  = reinterpret_cast<uint8_t*>(&result[0]);
  std::cerr << result << std::endl;
  std::string err;
  json11::Json json_obj = json11::Json::parse(result, err);
  if(err.empty() == false) {
    std::cerr << "ERROR: " <<  err.c_str() << std::endl;
  }
  const json11::Json &obj = json_obj["session_id"];
  if(obj.is_null()) {
    std::cerr << "ERROR: this packet is discarded because it does not convey any session IDs." << std::endl;
    return;
  }
  uint64_t session_id  = boost::lexical_cast<uint64_t>(obj.string_value());

  bool status = SessionManager::instance().append_payload(session_id, value, length);
  if(status == false) {
    std::cerr <<"ERROR: this packet is discarded because the corresponding session does not exist."<< std::endl;
  }

  ArribalInterests++;
    if(ArribalInterests == AllInterests){
      std::cerr << "send data" << std::endl;
      boost::system::error_code error;
      send_data(error,session_id);
      ArribalInterests = 0;
      AllInterests = 0;
    }

  return; 
}

void Producer::onData(const ndn::Interest&, const ndn::Data& data){
    const ndn::Block wire   = data.getContent();
    const size_t         length = wire.value_size();
    const uint8_t*       value  = wire.value();

    std::string str;
    std::copy(value, value + length, std::back_inserter(str));

    std::string err;
    json11::Json json_obj = json11::Json::parse(str, err);
    if(err.empty() == false) {
      std::cerr <<"ERROR: " << err.c_str()<< std::endl;
    }
    const json11::Json &obj = json_obj["session_id"];
    if(obj.is_null()) {
      std::cerr <<"ERROR: this packet is discarded because it does not convey any session IDs."<< std::endl;
      return;
    }
    uint64_t session_id  = boost::lexical_cast<uint64_t>(obj.string_value());

    bool status = SessionManager::instance().append_payload(session_id, value, length);
    if(status == false) {
      std::cerr <<"ERROR: this packet is discarded because the corresponding session does not exist."<< std::endl;
    }
  
    ArribalInterests++;
   // std::cerr << ArribalInterests << " " << AllInterests << std::endl;
    if(ArribalInterests == AllInterests){
      std::cerr << "send data" << std::endl;
      boost::system::error_code error;
      send_data(error,session_id);
      ArribalInterests = 0;
      AllInterests = 0;
    }
  return;
}


void Producer::onNack(const ndn::Interest&, ndn::lp::Nack& nack){
  std::cerr << "Received Nack with reason " << nack.getReason() << std::endl; 
}

void Producer::onTimeout(const ndn::Interest& interest){
  std::cerr << "Timeout for  " << interest << std::endl; 
}

void Producer::send_data(const boost::system::error_code& error, uint64_t session_id) {
  try {
    const std::vector<uint8_t> &buffer      = SessionManager::instance().buffer(session_id);
    std::shared_ptr<ndn::Data>  data_packet = SessionManager::instance().data_packet(session_id);
    data_packet->setContent(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());
    this->m_ndn_face.put(*data_packet);
    std::cerr << "[INFO] Sent a data packet " <<  data_packet->getName().toUri().c_str() << std::endl;
    SessionManager::instance().dump(std::cerr);
    SessionManager::instance().erase(session_id);
  }
  catch(std::out_of_range&) {
    std::cerr << "ERROR: Session does not exist." << std::endl;
  }
  return;
}

void Producer::onRegisterFailed(const ndn::Name& prefix, const std::string& reason) {
  std::cerr << "ERROR: Failed to register prefix \"" << prefix << "\" in local hub's daemon ("
            << reason << ")" << std::endl;
  m_ndn_face_ptr->shutdown();
}



 std::vector<std::string> Producer::convertInterestName(const std::string &interest_name) {
  std::vector<std::string> token_list;
  boost::algorithm::split(token_list, interest_name, boost::is_any_of("/"));

  std::string keyword(token_list.back());
  token_list.pop_back();

  std::string function_name(token_list.back());
  token_list.pop_back();
  std::string routable_prefix(boost::algorithm::join(token_list, "/"));

  std::cerr << "Parse results:" << std::endl
            << "routable: " << routable_prefix << std::endl
            << "function: " << function_name << std::endl
            << "keywords: " << keyword << std::endl;


  std::vector<std::string> key_list;
  boost::algorithm::split(key_list, keyword, boost::is_any_of(" "));
  std::string target(key_list.back());
  key_list.pop_back();
  std::string location_key(key_list.back());

  std::vector<std::string> lockey_list;
  boost::algorithm::split(lockey_list, location_key, boost::is_any_of(":"));
  std::string location(lockey_list.back());
  boost::algorithm::replace_all(location, "[", "");
  boost::algorithm::replace_all(location, "]", "");

  std::vector<std::string> loc_list;
  boost::algorithm::split(loc_list, location, boost::is_any_of(","));

  std::vector<std::string> locname_list;
  for(unsigned int i = 0; i < loc_list.size();i++){
    std::vector<std::string> tmp;

    for (unsigned int j = 0; j < loc_list[i].size(); j += 1){
      tmp.push_back(loc_list[i].substr(j,1));
    }
    std::string loc_name("/" + boost::algorithm::join(tmp, "/"));
     
    locname_list.push_back(loc_name);
  };

  std::vector<std::string> Interest_list;
  for(unsigned int i = 0; i < locname_list.size(); i++){
      std::string Interest_name(locname_list[i]+ "/" 
                                + function_name + "/"
                                + "#a:[" + loc_list[i] + "] " 
                                + target);
      std::cerr << "Interest name: " << Interest_name << std::endl;
      Interest_list.push_back(Interest_name);
  }

  return Interest_list;
}

std::vector<std::string> Producer::ExtractTargets(const std::string &interest_name){
    std::vector<std::string> token_list;
    boost::algorithm::split(token_list,interest_name,boost::is_any_of("/"));

    //token_list.pop_back();
    std::string keyword(token_list.back());
    std::vector<std::string> key_list;
    boost::algorithm::split(key_list,keyword,boost::is_any_of(" "));
    
    std::string material(key_list.back());
    std::vector<std::string> material_list;
    boost::algorithm::split(material_list,material,boost::is_any_of(":"));

    std::string target(material_list.back());
    boost::algorithm::replace_all(target,"[","");
    boost::algorithm::replace_all(target,"]","");

    std::vector<std::string> target_list;
    boost::algorithm::split(target_list,target,boost::is_any_of(","));

    return target_list;
}

 std::vector<std::string> Producer::ExtractLocname(const std::string &interest_name) {//冗長
 std::cerr << interest_name << std::endl;
  std::vector<std::string> token_list;
  boost::algorithm::split(token_list, interest_name, boost::is_any_of("/"));

  token_list.pop_back();
  std::string keyword(token_list.back());
  token_list.pop_back();

  std::string function_name(token_list.back());
  token_list.pop_back();
  std::string routable_prefix(boost::algorithm::join(token_list, "/"));

  std::vector<std::string> key_list;
  boost::algorithm::split(key_list, keyword, boost::is_any_of(" "));
  std::string target(key_list.back());
  key_list.pop_back();
  std::string location_key(key_list.back());

  std::vector<std::string> lockey_list;
  boost::algorithm::split(lockey_list, location_key, boost::is_any_of(":"));
  std::string location(lockey_list.back());
  boost::algorithm::replace_all(location, "[", "");
  boost::algorithm::replace_all(location, "]", "");

  std::vector<std::string> loc_list;
  boost::algorithm::split(loc_list, location, boost::is_any_of(","));

  return loc_list;
 }

std::string Producer::decodeURI(const std::string &uri){
    std::string replaced_string(uri);

    boost::algorithm::replace_all(replaced_string, "%23", "#");
    boost::algorithm::replace_all(replaced_string, "%3A", ":");
    boost::algorithm::replace_all(replaced_string, "%2C", ",");
    boost::algorithm::replace_all(replaced_string, "%5B", "[");
    boost::algorithm::replace_all(replaced_string, "%5D", "]");
    boost::algorithm::replace_all(replaced_string, "%20", " ");
    boost::algorithm::replace_all(replaced_string, "%27", "'");
    boost::algorithm::replace_all(replaced_string, "%28", "(");
    boost::algorithm::replace_all(replaced_string, "%29", ")");
    boost::algorithm::replace_all(replaced_string, "%2A", "*");

    return replaced_string;
}

template <class F>
void Producer::post_task(F f) {
  this->io_service().post(f);
  return;
}

boost::asio::io_service& Producer::io_service() {
  ++m_current_queue;
  m_current_queue %= m_current_queue % m_num_queue;
  return m_io_service_pool.at(m_current_queue);
}

