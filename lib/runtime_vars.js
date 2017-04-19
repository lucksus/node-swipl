const execSync = require('child_process').execSync;

// Runs swipl to extract runtime variables used on
// this platform.

exports.dump = () => {
    const output = execSync('swipl --dump-runtime-variables=sh').toString();
    const vars = {};
    for (const line of output.split(/\r?\n/)) {
        const match = line.match(/^([^=]+)="([^"]+)"/);
        if (match) {
            vars[match[1]] = match[2];
        }
    }
    return vars;
};
