#include "visuals/application.hpp"

#include "debug.hpp"


typedef boost::log::trivial::severity_level loglevel_t;
BOOST_LOG_ATTRIBUTE_KEYWORD(expr_named_scope, "Scope", boost::log::attributes::named_scope::value_type);
BOOST_LOG_ATTRIBUTE_KEYWORD(expr_origin, "Origin", boost::log::attributes::constant<std::string>::value_type)
/**
 * @brief Logging formatter for the life view of this program.
 *
 * @param record_view "array" of all expression values
 * @param stream logging format stream
 */
void logFormatter(const boost::log::record_view &record_view, boost::log::basic_formatting_ostream<char> &stream)
{
  auto named_scope = record_view[expr_named_scope];
  stream << '[' << record_view[expr_origin] << "::" << named_scope.get().back().scope_name << "] ";

  auto severity = record_view[boost::log::trivial::severity];
  switch (severity.get())
  {
  case loglevel_t::debug:
    stream << "\033[0;94m[";
    break;
  case loglevel_t::info:
    stream << "\033[0;92m[";
    break;
  case loglevel_t::warning:
    stream << "\033[0;93m[";
    break;
  case loglevel_t::error:
  case loglevel_t::fatal:
    stream << "\033[0;91m[";
    break;
  default:
    stream << '[';
    break;
  };
  stream << severity << "] \t"
         << record_view[boost::log::expressions::smessage] << "\033[0m";
}

void initLogging(boost::log::trivial::severity_level log_level)
{
  typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;
  boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

  boost::shared_ptr<std::ostream> logging_stream{&std::clog, boost::null_deleter{}};
  sink->locked_backend()->add_stream(logging_stream);

  sink->set_formatter(&logFormatter);

  boost::log::core_ptr core = boost::log::core::get();
  core->add_sink(sink);
  core->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
  core->add_global_attribute("Scope", boost::log::attributes::named_scope());
  core->set_filter(boost::log::trivial::severity >= log_level);
}


int main(int argc, char *argv[]) {
  initLogging(boost::log::trivial::severity_level::trace);

  Application app;
  return app.run(argc, argv);
}