# News Watcher

A Linux C++ service that watches the U.S. Bureau of Labor Statistics (BLS) Consumer Price Index (CPI) release and notifies connected TCP clients when a new monthly CPI publication appears.

## Sources

The default configuration polls four BLS endpoints:

- CPI HTML release
- BLS public API
- BLS RSS feed
- CPI PDF release

These are different representations of the same BLS CPI data, not independent news providers.

## Behaviour

Each listener records the first published month it sees. When a later month is detected, it extracts the CPI value and broadcasts a fixed 32-byte message to all TCP clients connected to port `11111` (`17111` in `--test` mode). The message contains a timestamp, CPI value, and source identifier such as `cpi/api` or `cpi/rss`.

The application does not place trades, store news, or make trading decisions. It only detects the release and sends the event onward. PDF change detection exists, but CPI value extraction from PDF is not implemented.

## Build and run

The service targets Linux and requires CMake, a C++23 compiler, libcurl, cJSON, fmt, and the P7 logging library.

```bash
mkdir build && cd build
cmake ..
sudo make
/bin/news_watcher
```

Configuration is read from `config.json` by default. Use `--config /path/to/config.json` to supply another file, `--test` for test mode, and `--port <port>` to override the TCP port.

