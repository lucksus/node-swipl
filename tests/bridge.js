const assert = require('assert');
const swipl = require('../');

if (swipl.initialise() !== 1) {
    throw new Error('Failed to initialise SWI-Prolog.');
}

describe('SWIPL interface', () => {

    it('should return PL_NIL as string []', () => {
        const List = swipl.callPredicate('length(List, 0)').List;
        assert.equal(List, '[]');
    });

    it('should return a string for a prolog string', () => {
        const string = swipl.callPredicate("atom_string('a', String)").String;
        assert.equal(string, 'a');
    });

    it('should return null for unbound variable', () => {
        const unbound = swipl.callPredicate('A = _').A;
        assert.ok(unbound === null);
    });

    it('should return a compound term', () => {
        const compound = swipl.callPredicate('t(1,2,3,4) = C').C;
        assert.equal(compound.name, 't');
        assert.equal(compound.args.length, 4);
    });
});
