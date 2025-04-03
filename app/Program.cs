using System.Runtime.InteropServices;

try {
  string? unmanagedErr = null;
  GpuProcesses.Info[] procs;
  using (var p = new GpuProcesses(error => unmanagedErr = error)) {
    procs = p.ToArray();
  }
  if (unmanagedErr != null) {
     throw new Exception(unmanagedErr);
  }
  foreach (var p in procs) {
    Console.WriteLine(
      $"{p.DeviceIndex} {p.Pid} {p.ProcessType} {p.UsedGpuMemory}");
  }
}
catch(Exception e) {
  Console.WriteLine("ERROR: " + e.Message);
}

// https://learn.microsoft.com/en-us/dotnet/api/system.idisposable
public class GpuProcesses: IDisposable {
  public delegate void ErrorCallback(string message);

  [DllImport("mylib", CallingConvention = CallingConvention.Cdecl)]
  static extern IntPtr RegisterErrorCallback(ErrorCallback? callback);
  [DllImport("mylib", CallingConvention = CallingConvention.Cdecl)]
  static extern IntPtr GetGpuProcesses(out int size);
  [DllImport("mylib", CallingConvention = CallingConvention.Cdecl)]
  static extern void FreeGpuProcesses(IntPtr data);

  [StructLayout(LayoutKind.Sequential)]
  public struct Info {
    public UInt32 Pid;
    public char ProcessType;
    public UInt64 UsedGpuMemory;
    public UInt32 DeviceIndex;
  }

  private IntPtr ptr;
  private int dataSize;

  public GpuProcesses(ErrorCallback errorCallback) {
    RegisterErrorCallback(errorCallback);
    ptr = GetGpuProcesses(out dataSize);
  }
  ~GpuProcesses() {
    Dispose(disposing: false);
  }

  public void Dispose() {
    Dispose(disposing: true);
    GC.SuppressFinalize(this);
  }

  protected virtual void Dispose(bool disposing) {
    if(ptr == IntPtr.Zero) {
      return;
    }
    FreeGpuProcesses(ptr);
    RegisterErrorCallback(null);
    ptr = IntPtr.Zero;
  }

  public Info[] ToArray() {
    var dataArray = new Info[dataSize];
    for (int i = 0; i < dataSize; i++) {
      dataArray[i] = Marshal.PtrToStructure<Info>(
        ptr + i * Marshal.SizeOf<Info>()
      );
    }
    return dataArray;
  }
}
