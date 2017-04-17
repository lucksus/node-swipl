const execSync = require('child_process').execSync;
const output = execSync('swipl --dump-runtime-variables=sh').toString();
let found = false;
for (const line of output.split(/\r?\n/)) {
    const match = line.match(/^PLBASE="([^"]+)"/);
    if (match) {
        process.stdout.write(match[1] + '\n', () => {
            process.exit();
        });
        found = true;
    }
}
if (!found) {
    process.exit(1);
}
