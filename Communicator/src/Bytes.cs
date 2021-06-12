using System;
using System.Linq;
using System.Runtime.InteropServices;

namespace Probability
{
    public class Bytes
    {
        public static byte[] Merge(byte[] bytes1, byte[] bytes2)
        {
            byte[] bytes = new byte[bytes1.Length + bytes2.Length];
            bytes1.CopyTo(bytes, 0);
            bytes2.CopyTo(bytes, bytes1.Length);
            return bytes;
        }
        
        public static byte[] AppendStructBytes(object structObj, byte[] origin)
        {
            int size = Marshal.SizeOf(structObj);
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(structObj, buffer, false);
                byte[] bytes = new byte[size + origin.Length];
                origin.CopyTo(bytes, 0);
                Marshal.Copy(buffer, bytes, origin.Length, size);
                return bytes;
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }

        public static byte[] StructToBytes(object structObj)
        {
            int size = Marshal.SizeOf(structObj);
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(structObj, buffer, false);
                byte[] bytes = new byte[size];
                Marshal.Copy(buffer, bytes, 0, size);
                return bytes;
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }
    }
}