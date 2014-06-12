A java agent to generate /tmp/perf-<pid>.map files for JITted methods for use with `perf`

## Build

    cmake .
    make

## Use

Add `-agentpath:<dir>/libperfmap.so` to the java command line.


## Disclaimer

I'm not a professional C code writer. The code is very "experimental", and it is e.g. missing checks for error conditions etc.. Use it at your own risk. You have been warned!
