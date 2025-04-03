#include <cstdio>
#include "core.h"

const int Initial_Count = 10;
const int MaxRetry = 10;

ErrorCallback error_callback = nullptr;

#ifndef NVML_API_VERSION
#define NVML_SUCCESS 0
int nvmlErrorString(int a) {return a;}
#endif // NVML_API_VERSION

extern "C" {
  void RegisterErrorCallback(ErrorCallback callback) {
    error_callback = callback;
  }

  GpuProcessInfo* GetGpuProcesses(int* size) {
    try {
      return GetGpuProcessesInternal(size);
    }
    catch (const std::exception& e) {
      if (error_callback) {
        error_callback(e.what());
      }
      return nullptr;
    }
  }

  void FreeGpuProcesses(GpuProcessInfo* data) {
    std::unique_ptr<GpuProcessInfo[]> buf(data);
  }
}
