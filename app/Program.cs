using System.Runtime.InteropServices;

[DllImport("mylib", CallingConvention = CallingConvention.Cdecl)]
static extern IntPtr GetGpuProcesses(out int size);
[DllImport("mylib", CallingConvention = CallingConvention.Cdecl)]
static extern void FreeGpuProcesses(IntPtr data);

int size;
var ptr = GetGpuProcesses(out size);
if (ptr == IntPtr.Zero) {
  throw new Exception("get_data returned null");
}
var dataArray = new GpuProcessInfo[size];
for (int i = 0; i < size; i++) {
  dataArray[i] = Marshal.PtrToStructure<GpuProcessInfo>(
    ptr + i * Marshal.SizeOf<GpuProcessInfo>()
    );
}
FreeGpuProcesses(ptr);

foreach (var data in dataArray) {
  Console.WriteLine(
    $"{data.DeviceIndex} {data.Pid} {data.ProcessType} {data.UsedGpuMemory}");
}

[StructLayout(LayoutKind.Sequential)]
struct GpuProcessInfo {
  public UInt32 Pid;
  public char ProcessType;
  public UInt64 UsedGpuMemory;
  public UInt32 DeviceIndex;
}
