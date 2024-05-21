#pragma once

#include <boost/log/sources/severity_logger.hpp>  // severity_logger
#include <boost/log/trivial.hpp>                  // severity level enum
#include <boost/log/sources/severity_feature.hpp> // BOOST_LOG_SEV macro

#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/named_scope.hpp>

#include <boost/log/sinks.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/file.hpp>


#ifdef MW_DEBUG

#define MW_DECLARE_LOGGER \
  boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;

#define MW_SET_CLASS_ORIGIN \
  this->logger.add_attribute("Origin", boost::log::attributes::constant<std::string>(__FUNCTION__));

#define MW_SET_FUNC_SCOPE \
  BOOST_LOG_FUNC()

#define MW_LOG(severity) \
  BOOST_LOG_SEV(logger, boost::log::trivial::severity_level::severity)

#define MW_LOG_INVALID_TILE \
  MW_LOG(error) << "invalid tile position"

#else

#define MW_DECLARE_LOGGER
#define MW_SET_CLASS_ORIGIN
#define MW_SET_FUNC_SCOPE
#define MW_LOG(severity) if (false) std::clog
#define MW_LOG_INVALID_TILE

#endif // defined(MW_DEBUG)