const fs = require('fs');
const path = require('path');
const assert = require('assert');
const term = require('./lib/term');
const swipl = require('./build/Release/libswipl');

// Set SWI_HOME_DIR based on install-time value.

const filename = path.join(__dirname, 'plbase.conf');
const plbase = fs.readFileSync(filename, 'utf8').trim();
process.env.SWI_HOME_DIR = plbase;

let initialised = false;
let auto = true;

// Allows to disable autoinitialization of
// the SWI-Prolog engine.

const autoInitialise = (initialise) => {
    auto = !!initialise;
};

let current = null;

class Query {
    constructor(query) {
        assert.equal(typeof query, 'string',
            'Query must be set as a string.');
        assert.ok(current === null,
            'There can be only one open query at a time.');
        if (!initialised) {
            if (auto) {
                if (!swipl.initialise('node')) {
                    throw new Error('SWI-Prolog engine initialization failed.');
                }
                initialised = true;
            } else {
                throw new Error('SWI-Prolog is not initialised.');
            }
        }
        this.internal = new swipl.InternalQuery(query);
        current = this;
    }

    next() {
        const bindings = this.internal.next();
        if (!bindings) {
            return false;
        } else {
            return extractBindings(bindings);
        }
    }

    close() {
        this.internal.close();
        current = null;
    }
};

// Helper to call single query.

const call = (query) => {
    const instance = new Query(query);
    try {
        const bindings = instance.next();
        return bindings;
    } finally {
        instance.close();
    }
};

// Extracts bindings from the option list.

const extractBindings = (list) => {
    const bindings = {};
    while (list !== '[]') {
        bindings[list.head.args[0]] = list.head.args[1];
        list = list.tail;
    }
    return bindings;
};

const initialise = () => {
    return swipl.initialise('node');
};

const cleanup = () => {
    return swipl.cleanup();
};

module.exports = {
    autoInitialise,
    Query,
    call,
    term,
    initialise,
    cleanup
};
