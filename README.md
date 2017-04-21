# node-swipl

A Node.js interface to the SWI-Prolog.

[![Build Status](https://travis-ci.org/rla/node-swipl.svg?branch=master)](https://travis-ci.org/rla/node-swipl)

Installation:

You need to have SWI-Prolog installed and `swipl` binary available in
`PATH` and compiler installed. See "Platform support" for operating system
support. The library requires Node.js version 7+ and SWI-Prolog 6+.

```
npm install swipl
```

Also see "Known issues" for currently unsolved problems.

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

### Create modules and specify context

Use module operator `:` to call code in different
modules. The default call assumes the `user` module.

```js
swipl.call('assert(mymodule:test(1))');
console.log(swipl.call('mymodule:test(X)'));
```

### Consult external files

Load code from external files. You might have
to set the working directory if you want to use relative paths.

```js
swipl.call('working_directory(_, prolog)');
swipl.call('consult(mycode)');
```

### Constructing safe queries

Queries with data requiring proper escaping can be constructed
by using helper functions from swipl.term.

Example:

```js
const swipl = require('./');
const { list, compound, variable, serialize } = swipl.term;

const escaped = serialize(
    compound('member', [
        variable('X'),
        list([1, 2, 3, 4])]));

console.log(swipl.call(escaped));
```

Blobs and dicts are not supported.

### Output term representation

Prolog terms in variable bindings are converted into
JavaScript objects under the following rules:

 * Integers are converted to numbers.
 * Floats are converted to numbers.
 * Atoms and strings are converted to strings.
 * Empty list is converted to string `[]`.
 * List head tail pair is converted to object `{ head, tail }` where
   `head` and `tail` are converted terms.
 * Compound term is converted to object `{ name, args }` where
   `name` is the compound functor name and `args` is the array
   of converted argument terms.
 * Blobs and dicts are not supported and will throw an error.

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

 * Unicode data cannot be exchanged.
 * Exporting PL_BLOB terms is not handled.
 * Exporting PL_DICT terms is not supported. It is not supported at all by SWI-Prolog
   foreign interface.
 * Installed files cannot be copied around on *nix. The linker has `libswipl` location
   specified absolutely in the binding object file. The location of `SWI_HOME_DIR` is
   determined install-time and written into the file `plbase.conf`.
 * Attempt to use native SWI packages leads to symbol lookup errors
   like `readutil.so: undefined symbol: PL_new_atom`.
 * Custom initialization parameters are not yet implemented.

## Development

A list of helpful resources:
 
 * SWI-Prolog Foreign Interface documentation: <http://www.swi-prolog.org/pldoc/man?section=foreign>
 * Node.js native addons: <https://nodejs.org/api/addons.html>
 * PySWIP sources: <https://code.google.com/archive/p/pyswip/>

## Alternatives

 * [Pengines package][pengines-package].
 * Run SWI as HTTP server and create a JSON API.

[pengines-package]:https://www.npmjs.com/package/pengines

## Authors

Please see the AUTHORS file.

## License

Licensed under LGPL 3.0. A copy is available in [the LICENSE.txt file](LICENSE.txt).
File `lib/serialize_string.js` is ported from the [pengines][pengines] project and is licensed
under BSD (see the file header).

[pengines]:https://github.com/SWI-Prolog/pengines
