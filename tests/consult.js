const assert = require('assert');
const swipl = require('../');

describe('Consult files', () => {
    it('should load a trivial file', () => {
        swipl.callPredicate('consult(tests/consult)');
        assert.equal(swipl.callPredicate('hello(W)').W, 'world');
    });
});
