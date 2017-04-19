const assert = require('assert');
const swipl = require('../');

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

    it('should throw error for invalid input', () => {
        try {
            swipl.callPredicate('member(X, 1,2,3,4');
        } catch (err) {
            assert.ok(err.toString().indexOf('Syntax error')) > 0;
        }
    });

    it('should throw error for thrown error', () => {
        try {
            swipl.callPredicate('throw(error(test))');
        } catch (err) {
            assert.ok(err.toString().indexOf('Unknown message: error(test)')) > 0;
        }
    });
});
