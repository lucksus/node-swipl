const assert = require('assert');
const swipl = require('../');

if (swipl.initialise() !== 1) {
    throw new Error('Failed to initialise SWI-Prolog.');
}

describe('Consult files', () => {
    it('should load a trivial file', () => {
        const user = swipl.module('user');
        user.call_predicate('consult', ['tests/consult']);
        assert.ok(user.call_predicate('hello', ['W']).W, 'world');
    });
});
