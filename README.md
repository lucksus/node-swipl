# node-swipl

A Node.js interface to the SWI-Prolog.

[![Build Status](https://travis-ci.org/rla/node-swipl.svg?branch=master)](https://travis-ci.org/rla/node-swipl)

## Usage

Calling a predicate and returning bindings with
the first solution:

```js
const swipl = require('swipl');
const ret = swipl.call('member(X, [1,2,3,4])');
if (ret) {
    console.log(`Variable X value is: ${ret.X}`);
} else {
    console.log('Call failed.');
}
```

Outputs:

```
Variable X value is: 1
```

Calling a predicate and returning all solutions:

```js
const swipl = require('swipl');
const query = new swipl.Query('member(X, [1,2,3,4])');
let ret = null;
while (ret = query.next()) {
    console.log(`Variable X value is: ${ret.X}`);
}
```

Outputs:

```
Variable X value is: 1
Variable X value is: 2
Variable X value is: 3
Variable X value is: 4
```

There can be only one query open at a time.

### Error handling

Syntax errors in queries are thrown. Error messages
are read from the prolog. The current query is automatically
closed.

Invalid query example: `member(X, [1,2,3,4]` (missing closing paren):

```js
swipl.call('member(X, [1,2,3,4]');
```

Throws error with message:

```
Error: Error during query execution. Syntax error:
Operator expected
member(X, 1,2,3,4
** here **
```

Known errors are thrown with the error message.

```js
swipl.call('error:must_be(ground, _)');
```

Throws error with message:

```
Error: Error during query execution. Arguments are
not sufficiently instantiated.
```

Custom errors without a message are thrown with JavaScript
error containing the error term:

```js
swipl.call('throw(error(test))');
```

Throws error with message:

```
Error: Error during query execution. Unknown message: error(test).
```

### Disable autoinitialization

The embedded SWI-Prolog engine is automatically initialized
when creating the first query. This behavior can be disabled
by calling `swipl.autoInitialise(false)` before any query is
executed. The engine can be initialized later manually with
the `swipl.initialise()` function.

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

## Known issues

 * Exporting PL_BLOB terms is not handled.
 * Exporting PL_DICT terms is not supported. It is not supported at all by SWI-Prolog
   foreign interface.
 * Installed files cannot be copied around on *nix. The linker has libswipl location
   specified absolutely in the binding object file. The location of `SWI_HOME_DIR` is
   determined install-time and written into the file `plbase.conf`.

## Development

A list of helpful resources:
 
 * SWI-Prolog Foreign Interface documentation: <http://www.swi-prolog.org/pldoc/man?section=foreign>
 * Node.js native addons: <https://nodejs.org/api/addons.html>
 * PySWIP sources: <https://code.google.com/archive/p/pyswip/>

### License

Licensed under LGPL 3.0. A copy is available in [the LICENSE.txt file](LICENSE.txt).
