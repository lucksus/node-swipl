const assert = require('assert');
const swipl = require('../');

if (swipl.initialise() !== 1) {
    throw new Error('Failed to initialise SWI-Prolog.');
}

describe('Consult files', () => {
    it('should load a trivial file', () => {
        swipl.callPredicate('consult(tests/consult)');
        assert.equal(swipl.callPredicate('hello(W)').W, 'world');
    });
});
