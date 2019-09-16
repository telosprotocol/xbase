// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

namespace top
{
    namespace base
    {
        class xcompress_t
        {
        public://lossless but very fast compress and decompress,lz4 may get up 25% compress rate for txt/doc content at 600MB/s
            //Note: 1.) it is good for html/script/txt/doc,but useless for png/jpeg/video etc
            //Note: 2.) it is good for cache/use raw data of big sizes at real-time to reduce memory usage
            
            /*return : the number of bytes written into buffer 'dest' (necessarily <= maxOutputSize) or 0 if compression fails
             The larger the acceleration value of acceleration, the faster the algorithm, but also the lesser the compression.
             It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
             An acceleration value of "1" is default
             Values <= 0 will be replaced by 1.
             */
            static int lz4_compress(const char* source, char* dest, const int sourceSize, const int maxDestSize,int acceleration = 1);
            /*return : the number of bytes decompressed into destination buffer (necessarily <= maxDecompressedSize)
             If destination buffer is not large enough, decoding will stop and output an error code (<0).
             If the source stream is detected malformed, the function will stop decoding and return a negative result.
             This function is protected against buffer overflow exploits, including malicious data packets.
             It never writes outside output buffer, nor reads outside input buffer.
             */
            static int lz4_decompress(const char* source, char* dest, const int compressedSize, const int maxDecompressedSize);
            
            //note: the compressed size might bigger than inputSize
            static int lz4_get_compressed_bound_size(int inputSize); //calcuate the max size after compresessed  inputSize
        };
    }
};
