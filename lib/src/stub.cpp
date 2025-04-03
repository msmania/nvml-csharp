#ifdef USE_STUB

#include <vector>

#include "core.h"

struct GpuProcessInfo_Raw {
  unsigned int pid;
  unsigned long long usedGpuMemory;
};

GpuProcessInfo* GetGpuProcessesInternal(int* size) {
  std::vector<GpuProcessInfo> total_procs;
  for (unsigned int i = 0; i < 2; ++i) {
    unsigned int count = Initial_Count;
    auto procs = GetItems<GpuProcessInfo_Raw>(
      [](GpuProcessInfo_Raw buf[], unsigned int& count) -> bool {
        if (count < 1000) {
          count = 1000;
          return false;
        }
        count = 3;
        for (unsigned int i = 0; i < count; ++i) {
          buf[i] = {i + 1};
        }
        return true;
      },
      count,
      MaxRetry
    );
    for (unsigned int j = 0; j < count; ++j) {
      total_procs.push_back({
        procs[j].pid,
        char('A' + i),
        procs[j].usedGpuMemory,
        i
      });
    }
  }

  auto raw_buffer = std::make_unique<GpuProcessInfo[]>(total_procs.size());
  std::copy(total_procs.begin(), total_procs.end(), raw_buffer.get());
  *size = total_procs.size();
  return raw_buffer.release();
}

#endif // USE_STUB
