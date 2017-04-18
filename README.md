# node-swipl

A Node.js interface to the SWI-Prolog.

For now, this addon supports the creation of rules and facts on
the fly and querying.

## Platform support

The bindings are tested on various Linux distributions, on Windows,
and on MacOS. SWI-Prolog command `swipl` must be available in `PATH`
on all of these operating systems.

### Windows

Microsoft build tools must be installed:

```
npm install --global --production windows-build-tools
```

SWI-Prolog command `swipl` must be available in `PATH`.

### MacOS

SWI-Prolog must be installed or compiled through Macports. This is
described here <http://www.swi-prolog.org/build/macos.html>. The setup was
tested on MacOS Sierra by installing dependencies from ports and compiling
with prefix `/usr/local` (adjust `build.templ`).

### Basic usage

Initialization

    var swipl = require('swipl');
    swipl.initialise();

Create module

    var m = swipl.module("mymod");

Facts and rules (shortcut for `m.call_predicate("assert", [ term ])`)

    m.assert("likes(romeo, julia).");
    {}

Querying - Single solution

    m.call_predicate("likes", ["romeo", "X"]);
    { X: 'julia' }

Querying - Query

    m.assert("likes(john, julia).");
    {}
    var q = m.open_query("likes", ["X", "julia"]);
    undefined
    q.next_solution();
    { X: 'romeo' }
    q.next_solution();
    { X: 'john' }
    q.next_solution();
    false
    q.close();
    true

Cleanup

    swipl.cleanup();

### License

Licensed under LGPL 3.0. A copy is available in [the LICENSE.txt file](LICENSE.txt).
