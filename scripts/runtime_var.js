const assert = require('assert');
const runtimeVars = require('../lib/runtime_vars');

// Helper script to dump the requested SWI-Prolog
// runtime variable to the stdout.

const requested = process.argv[2];
assert.equal(typeof requested, 'string',
    'The name of the requested runtime variable must be set.');
const vars = runtimeVars.dump();
assert.ok(vars.hasOwnProperty(requested),
    'The requested runtime variable does not exist.');

process.stdout.write(vars[requested] + '\n');
