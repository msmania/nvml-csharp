#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
  struct GpuProcessInfo {
    unsigned int Pid;
    char ProcessType;
    unsigned long long UsedGpuMemory;
    unsigned int DeviceIndex;
  };
  EXPORT GpuProcessInfo* GetGpuProcesses(int* size);
  EXPORT void FreeGpuProcesses(GpuProcessInfo*);
}
