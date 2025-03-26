#include <cstdio>
#include <format>
#include <memory>
#include <stdexcept>
#include <vector>
#include <nvml.h>

#include "core.h"

#define CHECK(statement) \
  do { \
    auto result = (statement); \
    if (result != NVML_SUCCESS) { \
      throw std::runtime_error(std::format( \
        "`{0}` failed with {1} at {2}:{3}", \
        #statement, \
        nvmlErrorString(result), \
        __FILE__, \
        __LINE__ \
        ) \
      ); \
    } \
  } while (0)

const int MaxRetry_GetProcesses = 10;

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
  count = 0;
  for (int i = 0; i < MaxRetry_GetProcesses; ++i) {
    auto procs = std::make_unique<nvmlProcessInfo_t[]>(count);
    auto result =
      nvmlDeviceGetComputeRunningProcesses_v3(device, &count, procs.get());
    if (result == NVML_SUCCESS) {
      return procs;
    }
    if (result != NVML_ERROR_INSUFFICIENT_SIZE) {
      fprintf(
        stderr,
        "nvmlDeviceGetComputeRunningProcesses_v3 failed - %s\n",
        nvmlErrorString(result)
      );
      return nullptr;
    }
  }
  fprintf(stderr, "GetComputeProcesses giving up\n");
  return nullptr;
}

std::unique_ptr<nvmlProcessInfo_t[]> GetGraphicsProcesses(
  nvmlDevice_t device,
  unsigned int& count
) {
  count = 0;
  for (int i = 0; i < MaxRetry_GetProcesses; ++i) {
    auto procs = std::make_unique<nvmlProcessInfo_t[]>(count);
    auto result =
      nvmlDeviceGetGraphicsRunningProcesses_v3(device, &count, procs.get());
    if (result == NVML_SUCCESS) {
      return procs;
    }
    if (result != NVML_ERROR_INSUFFICIENT_SIZE) {
      fprintf(
        stderr,
        "nvmlDeviceGetGraphicsRunningProcesses_v3 failed - %s\n",
        nvmlErrorString(result)
      );
      return nullptr;
    }
  }
  fprintf(stderr, "GetGraphicsProcesses giving up\n");
  return nullptr;
}

extern "C" {

GpuProcessInfo* GetGpuProcesses(int* size) {
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

void FreeGpuProcesses(GpuProcessInfo* data) {
  std::unique_ptr<GpuProcessInfo[]> buf(data);
}

}
