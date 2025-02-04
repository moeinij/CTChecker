/*
 * See files AUTHORS and LICENSE for copyright details.
 */

#include <sstream>
#include <sys/resource.h>

#include "tchecker/algorithms/stats.hh"

namespace tchecker {

namespace algorithms {

void stats_t::set_start_time() { _start_time = std::chrono::steady_clock::now(); }

std::chrono::time_point<std::chrono::steady_clock> stats_t::start_time() const { return _start_time; }

void stats_t::set_end_time() { _end_time = std::chrono::steady_clock::now(); }

std::chrono::time_point<std::chrono::steady_clock> stats_t::end_time() const { return _end_time; }

double stats_t::running_time() const
{
  std::chrono::duration<double> duration = _end_time - _start_time;
  return duration.count();
}

long stats_t::max_rss() const
{
  struct rusage usage;
  int res = getrusage(RUSAGE_SELF, &usage);
  if (res == -1)
    return -1;
  return usage.ru_maxrss;
}

void stats_t::attributes(std::map<std::string, std::string> & m) const
{
  std::stringstream sstream;

  sstream << running_time();
  m["RUNNING_TIME_SECONDS"] = sstream.str();

  sstream.str("");
  sstream << max_rss();
  m["MEMORY_MAX_RSS"] = sstream.str();
}

} // end of namespace algorithms

} // end of namespace tchecker