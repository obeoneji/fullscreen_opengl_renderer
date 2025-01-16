#include <time.h>

struct CpuTimer {
  clock_t begin;
  clock_t end;

  CpuTimer() {
    begin = 0;
    end = 0;
  }

  ~CpuTimer() {}

  void Start() { begin = clock(); }

  void Stop() { end = clock(); }

  double Elapsed() {
    double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
    return elapsed;
  }
};
