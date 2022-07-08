#ifndef ERROR_TRACKER_HPP
#define ERROR_TRACKER_HPP

#include <mutex>

#include "config.hpp"

#define CIRCULAR_BUFFER_SIZE 128

/**
 * @brief the type should be specified in this manner: ERROR_<SUBMODULE>_<NAME>
 *        for example: ERROR_MQTT_UPARSABLE_JSON_MESSAGE
 * 
 */
enum class ErrorType{
  ERROR_MQTT_UNPARSABLE_JSON_MSG,
  ERROR_MQTT_BAD_TOPIC,
  ERROR_CONTROL_TOO_MUCH_PWR_REQUESTED,
  ERROR_CONTROL_WRONG_PWR_REQUEST,
  ERROR_MODBUS_READ,
  ERROR_MODBUS_WRITE,
  ERROR_CELLS_OVERTEMPERATURE,
  ERROR_CELLS_UNDERTEMPERATURE,
  ERROR_CELLS_OVERVOLTAGE,
  ERROR_CELLS_UNDERVOLTAGE
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

/**
 * @brief one instance should be created in the beginning of the program
 *        and the shared_ptr should be passed by class instance to instance
 *        everybody should have a shared_ptr of this error tracker 
 *        TODO: make this class singleton 
 */
class ErrorTracker{
public:
  ErrorTracker() : 
    m_debugBuffer(CIRCULAR_BUFFER_SIZE),
    m_infoBuffer(CIRCULAR_BUFFER_SIZE),
    m_warningBuffer(CIRCULAR_BUFFER_SIZE),
    m_fatalBuffer(CIRCULAR_BUFFER_SIZE)
  {}
  void PushBackError(const Error& error);
  const ErrorType PopError(const ErrorSeverityLevel& level);
  const ErrorSeverityLevel GetMaxSeverity() const;
private:
  ErrorSeverityLevel m_maxSeverityLevel;
  std::mutex m_dataMutex;
};

#endif //ERROR_TRACKER_HPP
