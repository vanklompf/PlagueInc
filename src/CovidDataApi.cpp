#include "CovidDataApi.h"
#include "UrlEncode.h"

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <memory>

static const int JSON_SIZE = 2*JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 4*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6);

struct JsonContainer
{
  JsonContainer(size_t size) : buf(size){}
  DynamicJsonDocument buf;
};

CovidDataApi::CovidDataApi(Print& logger)
: m_logger(logger),
  m_httpClient(new HTTPClient()),
  m_jsonContainer(new JsonContainer(JSON_SIZE)),
  m_lastTemp(0),
  m_isValid(false)
{
}

CovidDataApi::~CovidDataApi() = default;

void CovidDataApi::init()
{
  m_httpClient->setTimeout(2000);
  m_httpClient->setReuse(true);

  prepareUri();
}

void CovidDataApi::checkValidity()
{
  if (!m_isValid || (millis() - m_lastReadTime > MIN_READ_INTERVAL))
  {
    fetchWeather();
  }
}

void CovidDataApi::fetchWeather()
{
  m_logger.printf("m_uriBuffer: %d\n", sizeof(m_uriBuffer));
  m_logger.printf("YYYYYYYAHOO_WEATHER_URI: %d\n", std::strlen(CORONAVIRUS_TRACKER_URI));

  m_logger.println(__func__);

  m_httpClient->begin(CORONAVIRUS_TRACKER_HOST, 80, m_uriBuffer);
  m_httpClient->addHeader("Content-Type", "application/json");
  int httpCode = m_httpClient->GET();
  m_logger.printf("Http Status: %d\n", httpCode);

  uint32_t now = millis();
  if (httpCode == 200)
  {
    auto error = deserializeJson(m_jsonContainer->buf, m_httpClient->getString());

    //JsonObject& root = m_jsonContainer->buf.parseObject(m_httpClient->getString());
    m_httpClient->end();
    if (!error) 
    {
      const char* temp = m_jsonContainer->buf["query"]["results"]["channel"]["item"]["condition"]["temp"];
      m_logger.printf("Json temp: %s\n", temp);
      m_lastTemp = atoi(temp);
      m_lastReadTime = now;
      m_isValid = true;
      return;
    }
  }

  if (now - m_lastReadTime > MAX_VALIDITY)
  {
    m_logger.println("Invalidating temperature");
    m_isValid = false;
  }
}

void CovidDataApi::prepareUri()
{
  m_logger.println(__func__);
  url_encode(CORONAVIRUS_TRACKER_URI, m_uriBuffer);
}
