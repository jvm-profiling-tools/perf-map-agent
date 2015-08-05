A java agent to generate `/tmp/perf-<pid>.map` files for JITted methods for use with `perf`.

## Build

Make sure `JAVA_HOME` is configured properly.
Then run the following on the command line:

    cmake .
    make

## Use

Use `./perf-java <pid> <options>` to create a `perf-<pid>.map` file for the current state of the JVM and
run `perf top -p <pid>` for this process afterwards. `perf-java` expects `libperfmap.so` in the current directory.

Over time the JVM will JIT compile more methods and the `perf-<pid>.map` file will become stale. You need to
rerun `perf-java` to generate a new and current map.

Note: If `perf top` still doesn't show any detailled info it's probably because of a permission problem. A common solution is
to run the java program as a regular user with the agent enabled and then run a `chown root /tmp/perf-<pid>.map` to make
the file accessible for the root user. Then run `perf top` as root. The `perf-java` script tries to do that.

## Options

You can add a comma separated list of options to `perf-java` (or the `AttachOnce` runner). These options are currently supported:

 - `unfold`: Create extra entries for every codeblock inside a method that was inlined from elsewhere (named &lt;inlined_method&gt; in &lt;root_method&gt;).
    Be aware of the effects of 'skid' in relation with unfolding. See the section below.
 - `unfoldsimple`: similar to `unfold`, however, the extra entries do not include the " in &lt;root_method&gt;" part
 - `msig`: include full method signature in the name string
 - `dottedclass`: convert class signature (`Ljava/lang/Class;`) to the usual class names with segments separated by dots (`java.lang.Class`). NOTE: this currently breaks coloring when used in combination with [flamegraphs](https://github.com/brendangregg/FlameGraph).

## Skid

You should be aware that instruction level profiling is not absolutely accurate but suffers from
'[skid](http://www.spinics.net/lists/linux-perf-users/msg02157.html)'. 'skid' means that the actual instruction
pointer may already have moved a bit further when a sample is recorded. In that case, (possibly hot) code is reported at
an address shortly after the actual hot instruction.

If using `unfold`, perf-map-agent will report sections that contain code inlined from other methods as separate entries.
Unfolded entries can be quite short, e.g. an inlined getter may only consist of a few instructions that now lives inside of another
method's JITed code. The next few instructions may then already belong to another entry. In such a case, it is more likely that skid
will not only affect the instruction pointer inside of a method entry but may affect which entry is chosen in the first place.

Skid that occurs inside a method is only visible when analyzing the actual assembler code (as with `perf annotate`). Skid that
affects the actual symbol resolution to choose a wrong entry will be much more visible as wrong entries will be reported with
tools that operate on the symbol level like the standard views of `perf report`, `perf top`, or in flame graphs.

So, while it is tempting to enable unfolded entries for the perceived extra resolution, this extra information is sometimes just noise
which will not only clutter the overall view but may also be misleading or wrong.

## Disclaimer

I'm not a professional C code writer. The code is very "experimental", and it is e.g. missing checks for error conditions etc.. Use it at your own risk. You have been warned!

## License

This library is licensed under GPLv2. See the LICENSE file.

