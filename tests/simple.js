const assert = require('assert');
const swipl = require('../');

if (swipl.initialise() !== 1) {
    throw new Error('Failed to initialise SWI-Prolog.');
}

describe('Simple queries', () => {
    it('should call member', () => {
        assert.equal(swipl.callPredicate('member(M, [1,2,3,4])').M, 1);
    });
});
