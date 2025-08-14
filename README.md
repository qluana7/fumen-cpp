# fumen++

**fumen++** is a header only C++ library for encoding and decoding [fumen](https://fumen.zui.jp/) strings, which represent Tetris field states. This project is a C++ port of the original [knewjade/tetris-fumen](https://github.com/knewjade/tetris-fumen) (TypeScript) implementation.

## Features

- Encode and decode fumen strings to and from C++ objects
- Full support for Tetris field, piece operations, comments, and quiz mode
- Minimal dependencies (uses only the C++ standard library)
- Designed for C++17 and later

## Getting Started

### 1. Include in Your Project

Clone this repository and add the `fumen++` directory to your include path.

```shell
# On shell
git clone https://github.com/qluana7/fumen-cpp
```

```cmake
# CMakeLists.txt
target_include_directory(<target> PRIVATE <path_to_fumen++>)
```

### 2. Decoding a Fumen String

```cpp
#include <fumen.hpp>
#include <iostream>

int main() {
    std::string fumen_code = "v115@vhAAgH";
    auto pages = fumen::decode(fumen_code);

    for (const auto& page : pages) {
        std::cout << page.m_field.to_string() << std::endl;
        std::cout << "Comment: " << page.m_comment << std::endl;
    }
}
```

### 3. Encoding to a Fumen String

```cpp
#include <fumen.hpp>
#include <iostream>

int main() {
    fumen::fumen_page page;
    std::string fumen_code = fumen::encode(pages);

    std::cout << fumen_code << std::endl;
}
```

## References

- Original TypeScript implementation: [knewjade/tetris-fumen](https://github.com/knewjade/tetris-fumen)

## License

This project follows the same license as the original repository. See [knewjade/tetris-fumen](https://github.com/knewjade/tetris-fumen) for details.
