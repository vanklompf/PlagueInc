#pragma once

#include <memory>
#include <stdint.h>
#include <cstring>

class Print;
class HTTPClient;
struct JsonContainer;

class CovidDataApi final
{
public:
  CovidDataApi(Print& logger);
  ~CovidDataApi();
  void init();

  bool isValid() const {return m_isValid;}

  void operator=(const CovidDataApi&) = delete;

  struct CovidCases {
    int confirmed;
    int deaths;
    int recovered;
  };

  

private:
  const int MIN_READ_INTERVAL = 300000;
  const uint32_t MAX_VALIDITY = MIN_READ_INTERVAL * 6;

  static constexpr const char* CORONAVIRUS_TRACKER_HOST = "coronavirus-tracker-api.herokuapp.com";
  static constexpr const char* CORONAVIRUS_TRACKER_URI = "/v2/latest?country_code=pl";


  Print& m_logger;
  std::unique_ptr<HTTPClient> m_httpClient;
  std::unique_ptr<JsonContainer> m_jsonContainer;

  int m_lastTemp;
  uint32_t m_lastReadTime;
  bool m_isValid;
  char m_uriBuffer[std::strlen(CORONAVIRUS_TRACKER_URI) + 20];

  void fetchWeather();
  void checkValidity();
  void prepareUri();
};
