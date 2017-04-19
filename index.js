const assert = require('assert');
const swipl = require('./build/Release/libswipl');

const Query = exports.Query = class Query {
	constructor(query) {
		assert.equal(typeof query, 'string',
			'Query must be set as a string.');
		this.internal = new swipl.InternalQuery(query);
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
	}
};

// Helper to call single query.

exports.callPredicate = (query) => {
	const instance = new Query(query);
	const bindings = instance.next();
	instance.close();
	return bindings;
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

exports.initialise = function(prog) {
	if (!prog) {
		prog = 'node';
	}
	return swipl.initialise(prog);
};

exports.cleanup = function() {
	return swipl.cleanup();
};
