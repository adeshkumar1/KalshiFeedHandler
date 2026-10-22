# KalshiFeedHandler

Requires CMake 3.20+, a C++20 compiler, Boost 1.70+, and OpenSSL 3 development headers and libraries.
CMake downloads nlohmann/json 3.12.0 into the build directory during the first
configuration, which requires internet access. No system-wide JSON installation
is needed.

On macOS, install Boost and OpenSSL with Homebrew:

```sh
brew install boost openssl@3
cmake -S . -B build -DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)"
cmake --build build
```

On systems where OpenSSL is already discoverable, omit `-DOPENSSL_ROOT_DIR`.
The executable links OpenSSL's Crypto library for hashing, signing, encryption,
and private key loading. Include the headers you need, such as
`<openssl/evp.h>` and `<openssl/pem.h>`.

Boost.Beast and Boost.Asio headers are available to the executable, along with
OpenSSL's SSL library for secure WebSockets. For a TLS WebSocket client, include
`<boost/beast/websocket.hpp>`, `<boost/beast/websocket/ssl.hpp>`,
`<boost/beast/ssl.hpp>`, and `<boost/asio/ssl.hpp>`.

For JSON parsing and serialization, use:

```cpp
#include <nlohmann/json.hpp>

using json = nlohmann::json;
```
