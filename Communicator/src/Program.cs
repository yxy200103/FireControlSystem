using System;
using System.IO.Ports;
using System.Runtime.InteropServices;
using Probability;

namespace SerialPortControl
{
    class Program
    {
        class PointFrameData : IFrameData
        {
            public float X;
            public float Y;
            
            public byte[] ToBytes()
            {
                byte[] bytes = Bytes.StructToBytes(X);
                return Bytes.AppendStructBytes(Y, bytes);
            }
        }
        
        [DllImport("libdetector.so", EntryPoint = "detect")]
        static extern void Detect(ref float x, ref float y);
        
        static void Main(string[] args)
        {
            SerialPort port = new SerialPort();
            
            /* Begin: Try Open Serial Port */
            try
            {
                Console.WriteLine("Info: Opening serial port");
                ActionSyncRetry.Run(
                    actionName: "Open serial port",
                    action: () =>
                    {
                        String[] portNames = SerialPort.GetPortNames();
                        port.PortName = portNames[0];
                        port.BaudRate = 115200;
                        port.Open();
                        return port.IsOpen;
                    }
                );
            }
            catch (Exception exception)
            {
                Console.WriteLine(exception.Message + ", exiting");
                return;
            }
            /* End: Try Open Serial Port */

            Byte    SOF   = 0x05;
            UInt16  DLEN  = 8;
            Byte    SEQ   = 1;
            UInt16  CMD   = (ushort) Command.AimOffset;
            
            CommandFrame commandFrame = new CommandFrame(SOF, DLEN, SEQ, CMD, null);
            PointFrameData pointFrameData = new PointFrameData();
            
            /* Begin: Main Loop */
            while (true)
            {
                try
                {
                    Detect(ref pointFrameData.X, ref pointFrameData.Y);
                    commandFrame.Data = pointFrameData;
                    byte[] frameBytes = commandFrame.ToBytes();
                    port.Write(frameBytes, 0, frameBytes.Length);
                }
                catch (InvalidOperationException operationException)
                {
                    if (!port.IsOpen)
                    {
                        Console.WriteLine(operationException);
                        Console.WriteLine("Current port is closed, reopening...");
                        
                        /* Begin: Try Reopen Serial Port */
                        try
                        {
                            Console.WriteLine("Info: Repening serial port");
                            ActionSyncRetry.Run(
                                actionName: "Reopen serial port",
                                action: () =>
                                {
                                    port.PortName = SerialPort.GetPortNames()[0];
                                    port.Open();
                                    return port.IsOpen;
                                }
                            );
                        }
                        catch (Exception exception)
                        {
                            Console.WriteLine(exception.Message + ", exiting");
                            return;
                        }
                        /* End: Try Reopen Serial Port */
                        
                    } // if (!port.IsOpen)
                }
                catch (Exception exception)
                {
                    Console.WriteLine(exception);
                }
            } 
            /* End: Main Loop */
            
            port.Close();
        }
    }
}