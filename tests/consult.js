const assert = require('assert');
const swipl = require('../');

describe('Consult files', () => {
    it('should load a trivial file', () => {
        swipl.call('consult(tests/consult)');
        assert.equal(swipl.call('hello(W)').W, 'world');
    });
});
