# node-swipl

A Node.js interface to the SWI-Prolog.

## Usage

### Error handling

Syntax errors in queries are thrown. Error messages
are read from the prolog.

Invalid query example: `member(X, [1,2,3,4]` (missing closing paren):

```js
swipl.callPredicate('member(X, [1,2,3,4]');
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
swipl.callPredicate('error:must_be(ground, _)');
```

Throws error with message:

```
Error: Error during query execution. Arguments are
not sufficiently instantiated.
```

Custom errors without a message are thrown with JavaScript
error containing the error term:

```js
swipl.callPredicate('throw(error(test))');
```

Throws error with message:

```
Error: Error during query execution. Unknown message: error(test).
```

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

 * PL_BLOB is not handled.

### License

Licensed under LGPL 3.0. A copy is available in [the LICENSE.txt file](LICENSE.txt).
