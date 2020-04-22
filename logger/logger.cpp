#include "logger.hpp"

// Logger
void _logger( std::string message )
{
  std::cout << message << std::endl;
  syslog(LOG_NOTICE, "%s", message.c_str());
}