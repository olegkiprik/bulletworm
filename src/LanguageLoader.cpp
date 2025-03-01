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

#include "LanguageLoader.hpp"
#include <SFML/System/InputStream.hpp>
#include <bw_ext/Endianness.hpp>
#include <algorithm>

namespace Bulletworm {

////////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> LanguageLoader::loadFromStream(BackIterator wordInserter, sf::InputStream& stream) {
    std::vector<std::uint32_t> content;

    {
        std::int64_t contentByteSize = stream.getSize();
        content.resize((std::size_t)(contentByteSize / 4 + 1)); // + eof
        std::int64_t contentRead = stream.read(content.data(), contentByteSize);
        if (contentRead != contentByteSize)
            return "Language file reading failure";

        // endianness
        std::for_each(content.data(), content.data() + contentByteSize/sizeof(std::uint32_t),
                      [](std::uint32_t& v) {
                          v = n2hl(v);
                      });
    }

    std::size_t wordSize = 0;

    // ignore BOM
    for (std::size_t i = 1; i < content.size(); ++i) {
        // 0, \n, \r
        if (content[i] == 0 || content[i] == '\n' || content[i] == '\r') {
            content[i] = 0; // null-terminator!
            if (wordSize)
                wordInserter = (content.data() + i - wordSize);
            wordSize = 0;
        } else {
            ++wordSize;
        }
    }

    return {};
}

}