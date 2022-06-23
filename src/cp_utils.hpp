#ifndef CP_UTILS_HPP
#define CP_UTILS_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "json.hpp"

using json = nlohmann::json;

class JsonLoader {
private:
  json m_jsonConfig;
public:
  JsonLoader(){}
  bool LoadJsonFromFile(std::filesystem::path path)
  {
    std::ifstream ifs (path.string(), std::ifstream::in);
    if(ifs.is_open())
    {
      ifs >> m_jsonConfig;
    }
    else
      return false;
  }
  bool LoadJsonFromString(std::string jsonInput)
  {
    m_jsonConfig = json::parse(jsonInput);
  }
  json GetJsonConfig(){return m_jsonConfig;}
};
#endif //CP_UTILS_HPP
