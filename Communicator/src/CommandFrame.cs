using System;
using Probability;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
| Field | Offset   | Length (bytes) | Description                                          |
| ----- | -------- | -------------- | ---------------------------------------------------- |
| SOF   | 0        | 1              | Start of Frame, fixed to 0x05                        |
| DLEN  | 1        | 2              | Length of DATA, little-endian uint16_t               |
| SEQ   | 3        | 1              | Sequence number                                      |
| CRC8  | 4        | 1              | p = 0x31, init = 0xFF, reflect data &  remainder     |
| CMD   | 5        | 2              | Command, little-endian uint16_t                      |
| DATA  | 7        | DLEN           | Data                                                 |
| CRC16 | 7 + DLEN | 2              | p = 0x1021, init = 0xFFFF, reflect data  & remainder |
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace SerialPortControl
{
    public enum Command
    {
        GameStatus = 0x0001,
        GameResult = 0x0002,
        RobotHP = 0x0003,
        DartStatus = 0x0004, // avaliable since 2020
        ICRABuffDebuffZoneStatus = 0x0005, // avaliable since 2020
        AimOffset = 0x0006,
        FieldEvent = 0x0101,
        SupplyProjectileAction = 0x0102,
        SupplyProjectileRequest = 0x0103, // deprecated since 2020
        RefereeWarning = 0x0104,
        DartRemainTime = 0x0105, // avaliable since 2020
        RobotStatus = 0x0201,
        RobotPowerHeat = 0x0202,
        RobotPosition = 0x0203,
        RobotBuff = 0x0204,
        AerialRobotEnergy = 0x0205,
        RobotHurt = 0x0206,
        RobotShoot = 0x0207,
        BulletRemaining = 0x0208,
        RfidStatus = 0x0209, // avaliable since 2020
        DartClientCmd = 0x020A, // avaliable since 2020
        InteractionHeader = 0x0301
    };
    
    public interface IFrameData
    {
        public byte[] ToBytes();
    }

    public class CommandFrame
    {
        public static CRC Crc8  = new CRC(0x31,    8,   0xFF,   0x00);
        public static CRC Crc16 = new CRC(0x1021, 16, 0xFFFF, 0x0000);
        
        public  Byte       SOF;
        public  UInt16     DataLength;
        public  Byte       Sequence;
        public  Byte       CRC8Value;
        public  UInt16     CommandId;
        public  IFrameData Data;

        public CommandFrame(byte sof, ushort dataLength, byte sequence, ushort commandId, IFrameData data)
        {
            SOF = sof;
            DataLength = dataLength;
            Sequence = sequence;
            byte dlenLo = Convert.ToByte(dataLength & 0xFF);
            byte dlenHi = Convert.ToByte((dataLength >> 8) & 0xFF);
            ulong ulongCRC8Value = Crc8.Compute(new []{SOF, dlenLo, dlenHi, sequence});
            CRC8Value = Convert.ToByte(ulongCRC8Value);
            CommandId = commandId;
            Data = data;
        }
        
        public byte[] ToBytes()
        {
            byte[] bytes = new byte[0];
            bytes = Bytes.AppendStructBytes(SOF, bytes);
            bytes = Bytes.AppendStructBytes(DataLength, bytes);
            bytes = Bytes.AppendStructBytes(Sequence, bytes);
            bytes = Bytes.AppendStructBytes(CRC8Value, bytes);
            bytes = Bytes.AppendStructBytes(CommandId, bytes);
            bytes = Bytes.Merge(bytes, Data.ToBytes());
            ushort CRC16Value = Convert.ToUInt16(Crc16.Compute(bytes));
            bytes = Bytes.AppendStructBytes(CRC16Value, bytes);
            return bytes;
        }
    }
}