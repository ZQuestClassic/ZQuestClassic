// npx esbuild tests/buffer_entry.js --bundle > tests/buffer.js

import {Buffer} from 'buffer';

window.Buffer = Buffer;
