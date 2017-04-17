var swipl = require("./build/Release/libswipl");

function Module(mod) {
	this.module_name = mod;
}

Module.prototype = {
	module : function(mod) {
		return new Module(mod);
	},

	term_type : function(term) {
		var r = swipl.term_type(term);
		if(this.DEBUG) console.log("[libswipl] term_type: " + r + " " + term);
		return r;
	},

	open_query : function(predicate, args) {
		return new swipl.Query(predicate, args, this.module_name);
	},

	call_predicate : function(name, args) {
		var query = new swipl.Query(name, args, this.module_name);
		var result = false;
		var r = query.next_solution();
		if (r) {
			result = r;
			if(this.DEBUG) console.log("[libswipl] call_predicate: "
					+ JSON.stringify(r));
		} else {
			var ex = query.exception();
			if (ex) {
				throw new Error(ex.exc);
			} else {
				if(this.DEBUG) console.log("[libswipl] call_predicate: " + r);
			}
		}
		query.close();
		return result;
	},

	assert : function(term) {
		if(this.DEBUG) console.log("[libswipl] assert: " + term);
		return this.call_predicate("assert", [ term ]);
	}
};

exports.module = function(mod) {
	return new Module(mod);
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
