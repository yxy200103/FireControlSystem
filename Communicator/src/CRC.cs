
namespace Probability
{
    public class CRC
    {
        private readonly uint Polynomial;
        private readonly byte Width;
        private readonly uint InitialXOR;
        private readonly uint FinalXOR;
        private readonly bool DoReflectData;
        private readonly bool DoReflectRemainder;
        
        public CRC(uint polynomial, byte width, uint initialXor, uint finalXor, 
            bool doReflectData = true, bool doReflectRemainder = true)
        {
            Polynomial = polynomial;
            Width = width;
            InitialXOR = initialXor;
            FinalXOR = finalXor;
            DoReflectData = doReflectData;
            DoReflectRemainder = doReflectRemainder;
        }

        public ulong Compute(byte[] data)
        {
            ulong crcValue = InitialXOR;

            if (Width < 8)
            {
                foreach (var currentByte in data)
                {
                    byte dataByte = ReflectByte(currentByte);
                    for (byte bit = 8; bit > 0; bit--)
                    {
                        crcValue <<= 1;
                        if (((dataByte ^ crcValue) & GetTopBit()) > 0)
                        {
                            crcValue ^= Polynomial;
                        }
                        dataByte <<= 1;
                    }
                }
            }
            else
            {
                foreach (var currentByte in data)
                {
                    byte dataByte = ReflectByte(currentByte);
                    crcValue ^= (ulong) (dataByte << (Width - 8));
                    
                    for (byte bit = 8; bit > 0; bit--)
                    {
                        if ((crcValue & GetTopBit()) > 0)
                        {
                            crcValue = (crcValue << 1) ^ Polynomial;
                        }
                        else
                        {
                            crcValue <<= 1;
                        }
                    }
                }
            }

            crcValue &= GetCrcMask();

            if (Width < 8)
            {
                crcValue = (crcValue << (8 - Width));
            }

            return (ReflectRemainder(crcValue) ^ FinalXOR) & GetCrcMask();
        }
        
        private byte ReflectByte(byte dataByte)
        {
            if (DoReflectData)
            {
                byte reflection = 0x0;
                for (byte bit = 0; bit < 8; bit++)
                {
                    if ((dataByte & 1) == 1)
                    {
                        reflection |= (byte) (1 << (7 - bit));
                    }
                    
                    dataByte >>= 1;
                }

                return reflection;
            }
            
            return dataByte;
        }

        private ulong ReflectRemainder(ulong data)
        {
            if (DoReflectRemainder)
            {
                ulong reflection = 0;
                byte nbits = (byte) (Width < 8 ? 8 : Width);
                
                for (byte bit = 0; bit < nbits; bit++)
                {
                    if ((data & 1) == 1)
                    {
                        reflection |= 1ul << ((nbits - 1) - bit);
                    }

                    data >>= 1;
                }

                return reflection;
            }
            
            return data;
        }

        private ulong GetTopBit()
        {
            return (Width < 8) ? 1ul << 7 : 1ul << (Width - 1);
        }

        private ulong GetCrcMask()
        {
            return (Width < 8) ? (1ul << 8) - 1 : (1ul << Width) - 1;
        }
    }
}