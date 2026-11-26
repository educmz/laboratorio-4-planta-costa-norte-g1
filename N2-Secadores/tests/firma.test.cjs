const {test} = require('node:test');
const assert = require('node:assert/strict');
const {readFileSync} = require('node:fs');
const {join} = require('node:path');
const vm = require('node:vm');
const {webcrypto, createHmac} = require('node:crypto');

test('El JSON de prueba conserva el decimal firmado y distingue la muestra negativa', async () => {
  const html = readFileSync(join(__dirname, 'mqtt.html'), 'utf8');
  const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];
  const fields = new Map();
  const document = {getElementById(id) {
    if (!fields.has(id)) fields.set(id, {});
    return fields.get(id);
  }};
  const ctx = vm.createContext({document, TextEncoder, crypto:webcrypto});
  vm.runInContext(script, ctx);
  Object.assign(document.getElementById('gas'), {value:'400', valueAsNumber:400, checkValidity:()=>true});
  document.getElementById('ts').valueAsNumber = 1700000001;
  document.getElementById('key').value = 'secreto de prueba';
  const msg = await vm.runInContext('payload()', ctx);
  const firma = createHmac('sha256','secreto de prueba').update('secadores|1700000001|400.0').digest('hex').slice(0,16);
  assert.match(msg, /"valor":400\.0,/);
  assert.equal(JSON.parse(msg).firma, firma);
  document.getElementById('negative').checked = true;
  assert.notEqual(JSON.parse(await vm.runInContext('payload()', ctx)).firma, firma);
  assert.equal(await vm.runInContext("hmac(new Uint8Array(20).fill(11),'Hi There')",ctx),'b0344c61d8db3853');
});
