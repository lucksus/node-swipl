const fs = require('fs');
const path = require('path');
const runtimeVars = require('../lib/runtime_vars');

// Helper script to dump the requested SWI-Prolog
// runtime variable to a config file.

const filename = path.join(__dirname, '..', 'plbase.conf');
const vars = runtimeVars.dump();
fs.writeFileSync(filename, vars.PLBASE, 'utf8');
