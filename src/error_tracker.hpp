#ifndef ERROR_TRACKER_HPP
#define ERROR_TRACKER_HPP

#include <boost/circular_buffer.hpp>
#include "config.hpp"

#define CIRCULAR_BUFFER_SIZE 128

/**
 * @brief the type should be specified in this manner: ERROR_<SUBMODULE>_<NAME>
 *        for example: ERROR_MQTT_UPARSABLE_JSON_MESSAGE
 * 
 */
enum class ErrorType{
  ERROR_MQTT_UNPARSABLE_JSON_MSG,
};

enum class ErrorSeverityLevel{
  ERROR_DEBUG = 0,
  ERROR_INFO = 1,
  ERROR_WARNING = 2,
  ERROR_FATAL = 3
};

struct Error {
  ErrorType type;
  ErrorSeverityLevel level;
};

class ErrorTracker{
public:
  ErrorTracker() : 
    m_debugBuffer(CIRCULAR_BUFFER_SIZE),
    m_infoBuffer(CIRCULAR_BUFFER_SIZE),
    m_warningBuffer(CIRCULAR_BUFFER_SIZE),
    m_fatalBuffer(CIRCULAR_BUFFER_SIZE)
  {}
private:
  boost::circular_buffer<ErrorType> m_debugBuffer;
  boost::circular_buffer<ErrorType> m_infoBuffer;
  boost::circular_buffer<ErrorType> m_warningBuffer;
  boost::circular_buffer<ErrorType> m_fatalBuffer;
  ErrorSeverityLevel m_maxSeverityLevel;

  void PushBackError(const Error& error);
  const ErrorType PopError(const ErrorSeverityLevel& level);
  const ErrorSeverityLevel GetMaxSeverity() const;
};

#endif //ERROR_TRACKER_HPP
