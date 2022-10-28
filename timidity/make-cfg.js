const fs = require('fs');

function parseCfg(cfgPath) {
  const text = fs.readFileSync(cfgPath, 'utf-8');
  const cfg = { bank: [], drumset: [] };
  let currentMode = '';

  for (let line of text.split('\n')) {
    line = line.trim();
    if (line.startsWith('bank')) currentMode = 'bank';
    if (line.startsWith('drumset')) currentMode = 'drumset';
    if (line.match(/^\d+/)) {
      const [num, file] = line.split(/\s/);
      cfg[currentMode][Number(num)] = file;
    }
  }

  return cfg;
}

function writeCfg(cfg, cfgPath) {
  const lines = [];
  lines.push('drumset 0\n');
  for (let i = 0; i < cfg.drumset.length; i++) {
    if (cfg.drumset[i]) lines.push(`${i} ${cfg.drumset[i]}`);
  }
  lines.push('\nbank 0\n');
  for (let i = 0; i < cfg.bank.length; i++) {
    if (cfg.bank[i]) lines.push(`${i} ${cfg.bank[i]}`);
  }
  fs.writeFileSync(cfgPath, lines.join('\n'));
}

function makePpl160Cfg() {
  const ultraCfg = fs.readFileSync('ultra.cfg', 'utf-8');
  const lines = [];

  for (const line of ultraCfg.split('\n')) {
    if (line.startsWith('#')) continue;

    if (line.match(/^\d+/)) {
      const [num, file] = line.split(' ');

      if (!fs.existsSync(file)) {
        throw new Error('ultra.cfg is bad. ' + file);
      }

      const ppl160File = file.replace('ultra', 'ppl160');
      if (fs.existsSync(ppl160File)) {
        lines.push(`${num} ${ppl160File}`);
      } else {
        lines.push(`${num} ${file}`);
      }
    } else {
      lines.push(line);
    }
  }
}

function makeZcCfg() {
  const freepatsCfg = parseCfg('freepats.cfg');
  const ppl160Cfg = parseCfg('ppl160.cfg');
  const zcCfg = { bank: [], drumset: [] };

  for (let i = 0; i < ppl160Cfg.bank.length; i++) {
    zcCfg.bank[i] = freepatsCfg.bank[i] || ppl160Cfg.bank[i];
  }
  for (let i = 0; i < ppl160Cfg.drumset.length; i++) {
    zcCfg.drumset[i] = freepatsCfg.drumset[i] || ppl160Cfg.drumset[i];
  }

  console.log(zcCfg);
  writeCfg(zcCfg, 'zc.cfg');
}

// ppl160 is missing patch files so fall back to ultra when missing.
makePpl160Cfg();

// freepats sounds best, but doesn't have everything. fall back to ppl160 when missing.
makeZcCfg();

// imo the quality is: zc > ultra > ppl160 (but zc is just barely better than ultra)
