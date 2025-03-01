////////////////////////////////////////////////////////////
//
// Bulletworm - Advanced Snake Game
// Copyright (c) 2024 Oleh Kiprik (oleg.kiprik@proton.me)
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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <bw_ext/stream/FileOutputStream.hpp>

// the modification of SFML source files. Here's the notice:

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
void FileOutputStream::FileCloser::operator()(std::FILE* file) noexcept {
    std::fclose(file);
}


////////////////////////////////////////////////////////////
FileOutputStream::FileOutputStream() noexcept = default;


////////////////////////////////////////////////////////////
FileOutputStream::~FileOutputStream() noexcept = default;


////////////////////////////////////////////////////////////
FileOutputStream::FileOutputStream(FileOutputStream&& src) noexcept :
    m_file(std::move(src.m_file)) {}


////////////////////////////////////////////////////////////
FileOutputStream& FileOutputStream::operator=(FileOutputStream&& src) noexcept {
    if (this == &src)
        return *this;

    m_file = std::move(src.m_file);
    return *this;
}


////////////////////////////////////////////////////////////
bool FileOutputStream::open(const std::filesystem::path& filename) noexcept {

// when using MS Visual Studio
#if defined(_WIN32) && defined(_MSC_VER)

    std::FILE* fptr = nullptr;

    // Visual Studio tricks
    (void)_wfopen_s(&fptr, filename.c_str(), L"wb");
    m_file.reset(fptr);

#elif !defined(_WIN32)
    m_file.reset(std::fopen(filename.c_str(), "wb"));
#else
    #error not supported
#endif

    return m_file != nullptr;
}


////////////////////////////////////////////////////////////
std::int64_t FileOutputStream::write(const void* data, std::int64_t size) noexcept {
    if (m_file) {
        std::size_t ret{ std::fwrite(data, 1, (std::size_t)size, m_file.get()) };
        return (std::int64_t)ret;
    } else {
        return -1;
    }
}


////////////////////////////////////////////////////////////
std::int64_t FileOutputStream::seek(std::int64_t position) noexcept {
    if (m_file) {
        if (std::fseek(m_file.get(), (long)position, SEEK_SET))
            return -1;

        return tell();
    } else {
        return -1;
    }
}


////////////////////////////////////////////////////////////
std::int64_t FileOutputStream::tell() noexcept {
    if (m_file) {
        return std::ftell(m_file.get());
    } else {
        return -1;
    }
}


////////////////////////////////////////////////////////////
std::int64_t FileOutputStream::getSize() noexcept {
    if (m_file) {
        std::int64_t position = tell();
        std::fseek(m_file.get(), 0, SEEK_END);
        std::int64_t size = tell();

        if (seek(position) == -1)
            return -1;
        return size;
    } else {
        return -1;
    }
}

} // namespace Bulletworm
