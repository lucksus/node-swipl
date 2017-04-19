const assert = require('assert');
const swipl = require('../');

describe('Simple queries', () => {
    it('should call member', () => {
        assert.equal(swipl.callPredicate('member(M, [1,2,3,4])').M, 1);
    });
});
