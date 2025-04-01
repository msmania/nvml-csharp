#include <cstdio>
#include <vector>
#include <nvml.h>
#include "core.h"

const int Initial_Count = 10;
const int MaxRetry = 10;

ErrorCallback error_callback = nullptr;

class RAII_NVML {
 public:
  RAII_NVML() {
    CHECK(nvmlInit_v2());
  }
  ~RAII_NVML() {
    auto result = nvmlShutdown();
    if (result != NVML_SUCCESS) {
      fprintf(
        stderr,
        "nvmlShutdown failed - %s\n",
        nvmlErrorString(result)
      );
    }
  }
};

std::unique_ptr<nvmlProcessInfo_t[]> GetComputeProcesses(
  nvmlDevice_t device,
  unsigned int& count
) {
  count = Initial_Count;
  return GetItems<nvmlProcessInfo_t>(
    [device](nvmlProcessInfo_t buf[], unsigned int& count) -> bool {
      auto res = nvmlDeviceGetComputeRunningProcesses_v3(device, &count, buf);
      if (res == NVML_ERROR_INSUFFICIENT_SIZE) {
        return false;
      }
      CHECK(res);
      return true;
    },
    count,
    MaxRetry
  );
}

std::unique_ptr<nvmlProcessInfo_t[]> GetGraphicsProcesses(
  nvmlDevice_t device,
  unsigned int& count
) {
  count = Initial_Count;
  return GetItems<nvmlProcessInfo_t>(
    [device](nvmlProcessInfo_t buf[], unsigned int& count) -> bool {
      auto res = nvmlDeviceGetGraphicsRunningProcesses_v3(device, &count, buf);
      if (res == NVML_ERROR_INSUFFICIENT_SIZE) {
        return false;
      }
      CHECK(res);
      return true;
    },
    count,
    MaxRetry
  );
}

GpuProcessInfo* GetGpuProcessesInternal(int* size) {
  RAII_NVML nvml;

  unsigned int device_count;
  CHECK(nvmlDeviceGetCount(&device_count));

  nvmlReturn_t result;
  char name[NVML_DEVICE_NAME_BUFFER_SIZE];

  std::vector<GpuProcessInfo> total_procs;

  for (unsigned int i = 0; i < device_count; ++i) {
    nvmlDevice_t device;
    CHECK(nvmlDeviceGetHandleByIndex_v2(i, &device));

    result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
    if (result == NVML_SUCCESS) {
      printf("Device#%u: %s\n", i, name);
    } else {
      fprintf(
        stderr,
        "nvmlDeviceGetName failed - %s\n",
        nvmlErrorString(result)
      );
      continue;
    }

    unsigned int proc_count = 0;
    auto procs = GetComputeProcesses(device, proc_count);
    for (unsigned int j = 0; j < proc_count; ++j) {
      total_procs.push_back({
        procs[j].pid,
        'C',
        procs[j].usedGpuMemory,
        i
      });
    }
    procs = GetGraphicsProcesses(device, proc_count);
    for (unsigned int j = 0; j < proc_count; ++j) {
      total_procs.push_back({
        procs[j].pid,
        'G',
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
