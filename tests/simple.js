const assert = require('assert');
const swipl = require('../');

if (swipl.initialise() !== 1) {
    throw new Error('Failed to initialise SWI-Prolog.');
}

describe('Simple queries', () => {
    it('should call member', () => {
        const user = swipl.module('user');
        assert.equal(user.call_predicate('member', ['M', '[1,2,3,4]']).M, 1);
    });
});
