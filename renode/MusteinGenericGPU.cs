//
// Copyright (c) 2010-2018 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//
// Basic generic graphical controller without any 2D/3D acceleration
//
// v0.2 2018/11/29 anton.krug@microchip.com SanFrancisco Summit variant
// v0.3 2019/02/13 anton.krug@microchip.com More color modes, embedded memory and added versality

using Antmicro.Renode.Backends.Display;
using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Core.Structure.Registers;
using System.Collections.Generic;
using System;

namespace Antmicro.Renode.Peripherals.Video {
    [AllowedTranslations(AllowedTranslation.ByteToDoubleWord)] // allow byte aligned accesses on the whole peripheral
    public class MusteinGenericGPU : AutoRepaintingVideo, IDoubleWordPeripheral, IKnownSize {

        public MusteinGenericGPU(Machine machine, bool registers64bitAligned = false, bool lookupTableRgbx = true, 
                                 uint controlBit = 23) : base(machine) {
            
            this.machine = machine;
            this.lookupTableRgbx = lookupTableRgbx; // Allows to switch ordering for the 8bit mode depending what is closer to the native host colorspace
            this.controlBit = controlBit; // All acceses with this bit set are handled by the control registers
            this.controlOffset = 1L << (int)controlBit; // All acceses smaller than this value are going to the buffer
            this.is64bitAligned = registers64bitAligned; // Are control register aligned to 32bit or 64bit
            accessAligment = (registers64bitAligned) ? 8 : 4; // To what bytes the control registers are aligned
            sync = new object();

            colorTable = new uint[256];

            // Populating lookup table for the 8bit color mode to 24bit conversion
            for (uint index = 0; index < 256; index++) {
                uint red   = index >> 5; // masking it with 0xe0 before shift is redudant
                uint green = (index & 0x1C) >> 2;
                uint blue  = index & 0x3;
                uint value;
                if (lookupTableRgbx) {
                    value = red << 21 | green << 13 | blue << 6; // Converting RGB332 to RGB888 which will be used for RGBX8888
                }
                else {
                    value = red << 5 | green << 13 | blue << 22; // Converting RGB332 to BGR888 which will be used for BGRX8888
                }
                colorTable[index] = value;
            }

            // Populating transfer colorModeToPixelFormat
            colorModeToPixelFormatTable = new Dictionary<ColorMode, PixelFormat>() {
                {ColorMode.LowColor,  (lookupTableRgbx) ? PixelFormat.RGBX8888 : PixelFormat.BGRX8888},
                {ColorMode.HighColor, PixelFormat.RGB565},
                {ColorMode.TrueColor, PixelFormat.RGBX8888}
            };

            bufferBig = new byte[defaultWidth * defaultHeight * 4 * 2]; 
            Reconfigure(defaultWidth, defaultHeight, defaultColor);

            GenerateRegisterCollection();
        }

        public long Size => controlOffset * 2; // Peripheral is split into 2 equal partitions (buffer and control registers)

        private void Reconfigure(uint? setWidth = null, uint? setHeight = null, ColorMode? setColor = null) {

            PixelFormat finalFormat = Format;
            if (setColor != null) {
                // Change the color only if it's not null
                if (colorModeToPixelFormatTable.TryGetValue((ColorMode)setColor, out finalFormat)) {
                    colorMode = (ColorMode)setColor;
                } else {
                    this.Log(LogLevel.Error, "Setting wrong color value {0}, keeping original value {1}", setColor, finalFormat);
                }
            }

            lock (sync) {
                // use inherited Reconfigurator
                base.Reconfigure((int?)setWidth, (int?)setHeight, finalFormat);
                if (setWidth != Width || setHeight != Height) {
                    // If the size dimensions changed, do reallocate buffer for the worst case scenario, 8bytes per 8-bit pixel
                    // this way changing the packing mechanism and colormodes will not cause reallocating new buffers and 
                    // existing data will stay without the need to be copied/cloned between the buffers
                    bufferBig = new byte[Width * Height * 4 * 2]; 
                }
            }

            this.Log(LogLevel.Noisy, "The display is reconfigured to {0}x{1} with {2} color format",
                             Width, Height, Format.ToString());
        }

        private void GenerateRegisterCollection() {
            var registerDictionary = new Dictionary<long, DoubleWordRegister> {     

                // Register to change width
                {controlOffset + (long)Registers.Width * accessAligment, new DoubleWordRegister(this, defaultWidth)
                    .WithValueField(0,  16, name: "Width",
                    writeCallback: (_, x) => Reconfigure(setWidth:  x ), valueProviderCallback: _ => (uint)Width)
                },

                // Register to change height
                {controlOffset + (long)Registers.Height * accessAligment, new DoubleWordRegister(this, defaultHeight)
                    .WithValueField(0, 16, name: "Height",
                    writeCallback: (_, y) => Reconfigure(setHeight: y ), valueProviderCallback: _ => (uint)Height)
                },

                // Register to change color format and pixel mapping        
                {controlOffset + (long)Registers.Format * accessAligment, new DoubleWordRegister(this, (uint)defaultColor | ((uint)defaultPacking << 4))
                    .WithEnumField<DoubleWordRegister, ColorMode>(   0, 4, name: "Color",  writeCallback: (_, c) => Reconfigure(setColor: c))
                    .WithEnumField<DoubleWordRegister, PixelPacking>(4, 4, name: "Packing", writeCallback: (_, c) => pixelPacking = c)
                },

            };

            if (is64bitAligned) {
                // Dormant registers for the high 32bit accesses of the 64bit registers, 
                // just so there will not be logged any unimplemented accesses 

                foreach (long registerIndex in Enum.GetValues(typeof(Registers))) {
                    registerDictionary.Add(
                        controlOffset + registerIndex * accessAligment + 4, new DoubleWordRegister(this, 0x0)
                        .WithTag("Dormant upper 32bit of 64bit registers", 0, 32)
                    );
                }
            }

            registerHandler = new DoubleWordRegisterCollection(this, registerDictionary);
        }

        public void WriteDoubleWord(long address, uint value) {
            if (address >= controlOffset) {
                registerHandler.Write(address, value); // Handle it by control registers
            } else {
                // Write it into the memory buffer
                if (address >= bufferBig.Length) {
                    this.Log(LogLevel.Error, "Writing @{0} is outside boundaries", address);
                } else {
                    bufferBig[address++] = (byte)(value >> 24);
                    bufferBig[address++] = (byte)(value >> 16);
                    bufferBig[address++] = (byte)(value >> 8);
                    bufferBig[address++] = (byte)(value);
                }
            }
        }

        public uint ReadDoubleWord(long offset) {
            if (offset >= controlOffset) {
                return registerHandler.Read(offset); // Handle it by control registers
            } else {
                // Read it from the memory buffer
                if (offset >= bufferBig.Length) {
                    this.Log(LogLevel.Error, "Reading @{0} is outside boundaries", offset);
                    return 0;
                } else {
                    return (((uint)bufferBig[offset]) << 24) |
                                 (((uint)bufferBig[offset + 1]) << 16) |
                                 (((uint)bufferBig[offset + 2]) << 8) |
                                    ((uint)bufferBig[offset + 3]);
                }
            }
        }

        public override void Reset() {
            registerHandler.Reset();
        }

        protected override void Repaint() {
            Tuple<uint, uint> parameters;
            if (copyPatterns.TryGetValue(Tuple.Create(colorMode, is64bitAligned, pixelPacking), out parameters)) {
                lock (sync) {
                    CopyBuffers(colorMode == ColorMode.LowColor, parameters);
                }
            } else {
                // Shouldn't ever reach this point as all possible options are populated. And colorMode is checked for 
                // correct enum value when it's mutated
                this.Log(LogLevel.Error, "Unsuported colorMode ({0}, {1}, {2}), aligment and pixel packing is used", 
                         colorMode, is64bitAligned, pixelPacking);
            }
        }

        private void CopyBuffers(bool useLookup, Tuple<uint, uint> parameters) {
            uint bytesToCopy = parameters.Item1;
            uint bytesToSkip = parameters.Item2;

            if (useLookup) {
                
                // Each byte gets transfered via the lookup table (to transfer the 8bit 332 format to a format the backend can display
                if (bytesToSkip == 0) {
                    // We can transfer the whole lot of bytes without skipping bytes in the input buffer
                    for (int indexSrc = 0, indexDest = 0; indexDest < buffer.Length; indexSrc++) {
                        uint colorValue = colorTable[bufferBig[indexSrc]];
                        buffer[indexDest++] = 0;
                        buffer[indexDest++] = (byte)(colorValue);
                        buffer[indexDest++] = (byte)(colorValue >> 8);
                        buffer[indexDest++] = (byte)(colorValue >> 16);
                    }
                } else {
                    // When there are parts to skip and the copied bytes need conversion
                    uint indexDest = 0;
                    uint indexSrc  = 0;
                    while (indexDest < buffer.Length) {
                        for (uint indexPack = 0; indexPack < bytesToCopy; indexPack++, indexSrc++) {
                            uint colorValue = colorTable[bufferBig[indexSrc]];
                            buffer[indexDest++] = 0;
                            buffer[indexDest++] = (byte)(colorValue);
                            buffer[indexDest++] = (byte)(colorValue >> 8);
                            buffer[indexDest++] = (byte)(colorValue >> 16);
                        }
                        indexSrc += bytesToSkip;
                    }
                }
            }
            else {
                
                // No conversion between the bytes:
                if (bytesToSkip == 0) {
                    // We can copy the whole lot as it is
                    for (int index = 0; index < buffer.Length; index++) {
                        buffer[index] = bufferBig[index];
                    }
                } else {
                    // When there are parts to skip but no conversion needed
                    uint indexDest = 0;
                    uint indexSrc = 0;
                    while (indexDest < buffer.Length) {
                        for (uint indexPack = 0; indexPack < bytesToCopy; indexPack++, indexDest++, indexSrc++) {
                            buffer[indexDest] = bufferBig[indexSrc];
                        }
                        indexSrc += bytesToSkip;
                    }
                }
            }
        }

        private enum Registers : long { 
            // These will get multiplied by the accessAligment which is depending on the registers64bitAligned
            Width  = 0x0,
            Height = 0x1,
            Format = 0x2,
        }

        public enum PixelPacking: uint {
            SinglePixelPerWrite = 0,
            FullyPacked32bit    = 1,
            FullyPacked64bit    = 2
        }

        public enum ColorMode : uint {
            LowColor  = 1,  // 8-bit  per pixel, 3-bits Red, 3-bits Green and 2-bits Blue
            HighColor = 2,  // 16-bit per pixel, 5-bits Red, 6-bits Green and 5-bits Blue
            TrueColor = 3   // 32-bit per pixel, 8-bits Red, 8-bits Green and 8-bits Blue
        }

        // Default reset values of the control registers
        private readonly uint         defaultWidth   = 128;
        private readonly uint         defaultHeight  = 128;
        private readonly ColorMode    defaultColor   = ColorMode.TrueColor;
        private readonly PixelPacking defaultPacking = PixelPacking.SinglePixelPerWrite;

        private readonly Machine machine;
        private readonly int     accessAligment;
        private readonly bool    is64bitAligned;
        private readonly bool    lookupTableRgbx;
        private readonly uint    controlBit;
        private readonly long    controlOffset;
        private readonly uint[]  colorTable;

        private DoubleWordRegisterCollection registerHandler;
        private object                       sync;
        private byte[]                       bufferBig;
        private ColorMode                    colorMode;
        private PixelPacking                 pixelPacking;

        // Lookup tables
        private readonly Dictionary<ColorMode, PixelFormat> colorModeToPixelFormatTable;
        private readonly Dictionary<Tuple<ColorMode, bool, PixelPacking>, Tuple<uint, uint>> copyPatterns = new Dictionary<Tuple<ColorMode, bool, PixelPacking>, Tuple<uint, uint>>() {
            // <colors, is64bitAlignedPeripheral, packingMode> = <bufferCopyBytes, bufferSkipBytes>

            { Tuple.Create(ColorMode.LowColor,  false, PixelPacking.SinglePixelPerWrite), Tuple.Create(1u, 3u) },
            { Tuple.Create(ColorMode.HighColor, false, PixelPacking.SinglePixelPerWrite), Tuple.Create(2u, 2u) },
            { Tuple.Create(ColorMode.TrueColor, false, PixelPacking.SinglePixelPerWrite), Tuple.Create(4u, 0u) },

            { Tuple.Create(ColorMode.LowColor,  false, PixelPacking.FullyPacked32bit),    Tuple.Create(4u, 0u) },
            { Tuple.Create(ColorMode.HighColor, false, PixelPacking.FullyPacked32bit),    Tuple.Create(2u, 0u) },
            { Tuple.Create(ColorMode.TrueColor, false, PixelPacking.FullyPacked32bit),    Tuple.Create(4u, 0u) },

            // In a 32bit peripheral aligment mode using fully packed 64bit is ilegal and will act as fully packed 32bit
            { Tuple.Create(ColorMode.LowColor,  false, PixelPacking.FullyPacked64bit),    Tuple.Create(4u, 0u) }, 
            { Tuple.Create(ColorMode.HighColor, false, PixelPacking.FullyPacked64bit),    Tuple.Create(2u, 0u) },
            { Tuple.Create(ColorMode.TrueColor, false, PixelPacking.FullyPacked64bit),    Tuple.Create(4u, 0u) },

            { Tuple.Create(ColorMode.LowColor,  true,  PixelPacking.SinglePixelPerWrite), Tuple.Create(1u, 7u) },
            { Tuple.Create(ColorMode.HighColor, true,  PixelPacking.SinglePixelPerWrite), Tuple.Create(2u, 6u) },
            { Tuple.Create(ColorMode.TrueColor, true,  PixelPacking.SinglePixelPerWrite), Tuple.Create(4u, 4u) },

            { Tuple.Create(ColorMode.LowColor,  true,  PixelPacking.FullyPacked32bit),    Tuple.Create(4u, 4u) },
            { Tuple.Create(ColorMode.HighColor, true,  PixelPacking.FullyPacked32bit),    Tuple.Create(2u, 4u) },
            { Tuple.Create(ColorMode.TrueColor, true,  PixelPacking.FullyPacked32bit),    Tuple.Create(4u, 4u) },

            { Tuple.Create(ColorMode.LowColor,  true,  PixelPacking.FullyPacked64bit),    Tuple.Create(4u, 0u) },
            { Tuple.Create(ColorMode.HighColor, true,  PixelPacking.FullyPacked64bit),    Tuple.Create(2u, 0u) },
            { Tuple.Create(ColorMode.TrueColor, true,  PixelPacking.FullyPacked64bit),    Tuple.Create(4u, 0u) },

        };
    }
}
