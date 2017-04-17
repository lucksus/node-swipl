const execSync = require('child_process').execSync;
const output = execSync('swipl --dump-runtime-variables=sh').toString();
let found = false;
const vars = {};
for (const line of output.split(/\r?\n/)) {
    const match = line.match(/^([^=]+)="([^"]+)"/);
    if (match) {
        vars[match[1]] = match[2];
    }
}
const requested = process.argv[2];
process.stdout.write(vars[requested] + '\n');
