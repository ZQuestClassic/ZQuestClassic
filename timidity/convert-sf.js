const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const pathToUnsf = process.env.UNSF || `${__dirname}/../../../tools/unsf/build/unsf-static`;
for (const handle of fs.readdirSync(`${__dirname}/soundfonts`, { withFileTypes: true })) {
  if (!handle.isFile()) continue;

  console.log(`converting ${handle.name} to GUS pat files`);
  const sfPath = `${__dirname}/soundfonts/${handle.name}`;
  const outPath = `${__dirname}/soundfont-pats`;
  fs.mkdirSync(outPath, { recursive: true });
  execFileSync(pathToUnsf, [
    '-O', outPath,
    sfPath,
  ]);

  const sfName = path.parse(handle.name).name;
  const cfgPath = `${__dirname}/soundfont-pats/${sfName}.cfg`;
  const cfgText = fs.readFileSync(cfgPath, 'utf-8')
    .replace(new RegExp(`${sfName}/`, 'g'), `soundfont-pats/${sfName}/`)
    .replace(new RegExp(`${sfName}-Drums/`, 'g'), `soundfont-pats/${sfName}-Drums/`);
  fs.writeFileSync(cfgPath, cfgText);
}
