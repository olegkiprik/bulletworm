////////////////////////////////////////////////////////////
//
// Bulletworm - Advanced Snake Game
// Copyright (c) 2024-2025 Oleh Kiprik (oleg.kiprik@proton.me)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////

#ifndef FILE_OUTPUT_STREAM_HPP
#define FILE_OUTPUT_STREAM_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "OutputStream.hpp"
#include <filesystem>

// the modification of SFML source files

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

namespace Bulletworm {

////////////////////////////////////////////////////////////
// Simple output file stream
////////////////////////////////////////////////////////////
class FileOutputStream : public OutputStream {
public:

    // Creates no stream (call open() to use)
    FileOutputStream() noexcept;

    // Destructor
    ~FileOutputStream() noexcept override;

    // Deleted copying ctor
    FileOutputStream(const FileOutputStream&) = delete;

    // Deleted copying =
    FileOutputStream& operator=(const FileOutputStream&) = delete;

    // Moving ctor
    FileOutputStream(FileOutputStream&&) noexcept;

    // Moving =
    FileOutputStream& operator=(FileOutputStream&&) noexcept;

    // Returns true if succeed, otherwise false
    [[nodiscard]] bool open(const std::filesystem::path& filename) noexcept;

    // Returns -1 if error occured, otherwise the actual count of written bytes.
    // Param size in bytes
    [[nodiscard]] std::int64_t write(const void* data, std::int64_t size) noexcept override;

    // Returns -1 if error occured, otherwise the actual sought position
    // All is in bytes
    [[nodiscard]] std::int64_t seek(std::int64_t position) noexcept override;

    // Returns -1 if error occured, otherwise the told position (in bytes)
    [[nodiscard]] std::int64_t tell() noexcept override;

    // Returns -1 if error occured, otherwise the size of the stream in bytes
    std::int64_t getSize() noexcept override;

private:

    struct FileCloser {
        void operator()(std::FILE* file) noexcept;
    };

    std::unique_ptr<std::FILE, FileCloser> m_file; // stdio file stream
};

} // namespace Bulletworm

#endif // FILE_OUTPUT_STREAM_HPP
