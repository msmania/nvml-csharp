#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>

#ifndef NVML_API_VERSION
#define NVML_SUCCESS 0
int nvmlErrorString(int a) {return a;}
#endif // NVML_API_VERSION

#define CHECK(statement) \
  do { \
    auto result = (statement); \
    if (result != NVML_SUCCESS) { \
      std::ostringstream oss; \
      oss \
        << '`' \
        << #statement \
        << "` failed with " \
        << nvmlErrorString(result) \
        << " at " \
        << __FILE__ \
        << ':' \
        << __LINE__; \
      throw std::runtime_error(oss.str().c_str()); \
    } \
  } while (0)

template <typename T>
std::unique_ptr<T[]> GetItems(
  const std::function<bool(T[], unsigned int&)>& callback,
  unsigned int& count,
  int max_retry
) {
  for (int i = 0; i < max_retry; ++i) {
    // Allocate twice as much memory as needed in case the required buffer
    // has been increased after the previous call.
    auto buf = std::make_unique<T[]>(count * 2);
    if (callback(buf.get(), count)) {
      return buf;
    }
  }
  return nullptr;
}

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
  typedef void (*ErrorCallback)(const char*);
  struct GpuProcessInfo {
    unsigned int Pid;
    char ProcessType;
    unsigned long long UsedGpuMemory;
    unsigned int DeviceIndex;
  };
  EXPORT void RegisterErrorCallback(ErrorCallback);
  EXPORT GpuProcessInfo* GetGpuProcesses(int*);
  EXPORT void FreeGpuProcesses(GpuProcessInfo*);
}
