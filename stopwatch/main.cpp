#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

typedef uint64_t durationms_t;

// Get the time since epoch in milliseconds
durationms_t GetTimeMS()
{
  static const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

  const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

void DurationToString(std::ostream& o, durationms_t duration)
{
  o<<std::setfill('0')<<std::setw(2)<<((duration / 60000) % 3600)<<":"<<std::setfill('0')<<std::setw(2)<<((duration / 1000) % 60)<<":"<<std::setfill('0')<<std::setw(3)<<(duration % 1000);
}


// ** cStopWatch
//
// A stop watch that behaves like a real life stop watch, it has start, stop, reset and a total duration

class cStopWatch
{
public:
  cStopWatch();

  void Start();
  void Stop();
  void Reset();

  durationms_t GetTotalDurationMS() const;

private:
  bool running;
  durationms_t started;
  durationms_t totalDuration;
};

cStopWatch::cStopWatch() :
  running(false),
  started(0),
  totalDuration(0)
{
}

void cStopWatch::Start()
{
  assert(!running);

  // Started our stop watch
  started = GetTimeMS();

  running = true;
}

void cStopWatch::Stop()
{
  assert(running);

  // Get the time now
  const durationms_t now = GetTimeMS();

  // Add the duration for this period
  totalDuration += now - started;

  // Reset our start time
  started = 0;

  running = false;
}

void cStopWatch::Reset()
{
  started = 0;
  totalDuration = 0;

  running = false;
}

durationms_t cStopWatch::GetTotalDurationMS() const
{
  if (running) {
    // Get the time now
    const durationms_t now = GetTimeMS();

    // Return the previous duration plus the duration of the current period
    return totalDuration + (now - started);
  }

  return totalDuration;
}





// ** cTimeOut
//
// For use in a loop for example to work out when we something has expired

class cTimeOut {
public:
  explicit cTimeOut(durationms_t timeout);

  void Reset();

  bool IsExpired() const;

  durationms_t GetRemainingMS() const;

private:
  const durationms_t timeout;
  durationms_t startTime;
};

cTimeOut::cTimeOut(durationms_t _timeout) :
  timeout(_timeout),
  startTime(GetTimeMS())
{
}

void cTimeOut::Reset()
{
  startTime = GetTimeMS();
}

bool cTimeOut::IsExpired() const
{
  return ((GetTimeMS() - startTime) > timeout);
}

durationms_t cTimeOut::GetRemainingMS() const
{
  // Get the total time this timeout uhas been running for so far
  const int64_t duration = int64_t(GetTimeMS()) - int64_t(startTime);

  // Calculate the remaining time
  const int64_t remaining = (int64_t(timeout) - duration);

  // Return the remaining time if there is any left
  return (remaining >= 0) ? remaining : 0;
}


int main(int argc, char* argv[])
{
  // Just a little test of the stop watch and timeout

  cStopWatch stopWatch;

  // Create a 5 second time out
  cTimeOut timeout(5000);

  // Run through 3 5 second time outs
  for (size_t i = 0; i < 3; i++) {
    // Start a new time out
    timeout.Reset();

    // Start the stop watch
    stopWatch.Start();

    // Wait until the time out is expired
    while (!timeout.IsExpired()) {
      // Print out some debug information about the stop watch and time out
      std::cout<<"Stop watch time: ";
      DurationToString(std::cout, stopWatch.GetTotalDurationMS());
      std::cout<<", Timeout: "<<(timeout.IsExpired() ? "expired" : "not expired")<<", remaining: ";
      DurationToString(std::cout, timeout.GetRemainingMS());
      std::cout<<std::endl;

      // Sleep for 200 milliseconds (Avoids hogging the CPU and we don't have to update that frequently
      const uint64_t timeout_ms = 200;
      ::usleep(timeout_ms * 1000);
    }

    // Stop the stop watch
    stopWatch.Stop();

    // Print out some debug information about the stop watch and time out
    std::cout<<"Stop watch time: ";
    DurationToString(std::cout, stopWatch.GetTotalDurationMS());
    std::cout<<", Timeout: "<<(timeout.IsExpired() ? "expired" : "not expired")<<", remaining: ";
    DurationToString(std::cout, timeout.GetRemainingMS());
    std::cout<<std::endl;
    std::cout<<std::endl;
  }

  return EXIT_SUCCESS;
}
