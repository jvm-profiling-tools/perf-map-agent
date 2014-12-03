A java agent to generate /tmp/perf-<pid>.map files for JITted methods for use with `perf`

## Build

    cmake .
    make

## Use

Add `-agentpath:<dir>/libperfmap.so` to the java command line.

Note: If `perf top` still doesn't show any detailled info it's probably because of a permission problem. A common solution is
to run the java program as a regular user with the agent enabled and then run a `chown root /tmp/perf-<pid>.map` to make
the file accessible for the root user. Then run `perf top` as root. You could use the following script to do that:

```sh
sudo chown root /tmp/perf-$1.map
sudo perf top
```

## Disclaimer

I'm not a professional C code writer. The code is very "experimental", and it is e.g. missing checks for error conditions etc.. Use it at your own risk. You have been warned!

## License

This library is licensed under GPLv2. See the LICENSE file.
