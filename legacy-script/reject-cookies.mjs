// Watches OBS's embedded browser (CEF remote debugging) and clicks "Reject"
// on Twitch's cookie/advertising banner in any dock that shows it.
const PORT = Number(process.env.OBS_DEBUG_PORT) || 9222;
const POLL_MS = 2000;
const done = new Set();
let seenOBS = false, misses = 0;

const CLICK = `(() => {
  const accept = document.querySelector('[data-a-target="consent-banner-accept"]');
  if (!accept) return 'none';
  const isReject = b => b.innerText.trim() === 'Reject';
  let n = accept;
  while (n && ![...n.querySelectorAll('button')].some(isReject)) n = n.parentElement;
  const btn = n && [...n.querySelectorAll('button')].find(isReject);
  if (!btn) return 'no-reject';
  btn.click();
  return 'clicked';
})()`;

async function evalIn(page, expression) {
  const ws = new WebSocket(page.webSocketDebuggerUrl);
  await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
  const out = await new Promise(res => {
    ws.onmessage = e => { const m = JSON.parse(e.data); if (m.id === 1) res(m.result?.result?.value); };
    ws.send(JSON.stringify({ id: 1, method: 'Runtime.evaluate', params: { expression, returnByValue: true } }));
  });
  ws.close();
  return out;
}

while (true) {
  try {
    const pages = await (await fetch(`http://127.0.0.1:${PORT}/json`)).json();
    seenOBS = true; misses = 0;
    for (const p of pages.filter(p => p.type === 'page' && /(^|\.)twitch\.tv\//.test(new URL(p.url).host + '/'))) {
      const key = p.id + p.url;
      if (done.has(key)) continue;
      const r = await evalIn(p, CLICK).catch(() => null);
      if (r !== 'none') console.log(p.title, r);
      if (r === 'clicked') { done.add(key); console.log(new Date().toISOString(), 'rejected:', p.title); }
    }
  } catch {
    if (seenOBS && ++misses >= 5) process.exit(0); // OBS closed
  }
  await new Promise(r => setTimeout(r, POLL_MS));
}
