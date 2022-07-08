#include "error_tracker.hpp"

/*void ErrorTracker::PushBackError(const Error& error)
{
  switch(error.level)
  {
    case ErrorSeverityLevel::ERROR_DEBUG:
    {
      m_debugBuffer.push_back(error.type);
      break;
    }
    case ErrorSeverityLevel::ERROR_INFO:
    {
      m_infoBuffer.push_back(error.type);
      break;
    }
    case ErrorSeverityLevel::ERROR_WARNING:
    {
      m_warningBuffer.push_back(error.type);
      break;
    }
    case ErrorSeverityLevel::ERROR_FATAL:
    {
      m_fatalBuffer.push_back(error.type);
      break;
    }
  }
}

const ErrorType ErrorTracker::PopError(const ErrorSeverityLevel& level)
{
  ErrorType error;
  switch(level)
  {
    case ErrorSeverityLevel::ERROR_DEBUG:
    {
      error = m_debugBuffer[0];
      m_debugBuffer.pop_front();
      break;
    }
    case ErrorSeverityLevel::ERROR_INFO:
    {
      error = m_infoBuffer[0];
      m_infoBuffer.pop_front();
      break;
    }
    case ErrorSeverityLevel::ERROR_WARNING:
    {
      error = m_warningBuffer[0];
      m_warningBuffer.pop_front();
      break;
    }
    case ErrorSeverityLevel::ERROR_FATAL:
    {
      error = m_fatalBuffer[0];
      m_fatalBuffer.pop_front();
      break;
    }
  }
  
  m_maxSeverityLevel = m_maxSeverityLevel < level ? level : m_maxSeverityLevel;
  return error;
}

const ErrorSeverityLevel ErrorTracker::GetMaxSeverity() const
{
  size_t fatalSize = m_fatalBuffer.size();
  size_t warningSize = m_warningBuffer.size();
  size_t infoSize = m_infoBuffer.size();
  size_t debugSize = m_debugBuffer.size();
}*/
