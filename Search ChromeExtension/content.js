// Content script: highlight + next/prev/clear in *this* page.
(() => {
  const HIGHLIGHT = "mtf-highlight";
  const CURRENT = "mtf-current";

  // inject styles once
  if (!document.getElementById("mtf-style")) {
    const style = document.createElement("style");
    style.id = "mtf-style";
    style.textContent = `
      .${HIGHLIGHT}{background:#ffeb3b;color:#000;padding:0 2px;border-radius:2px}
      .${CURRENT}{background:#ff9800 !important}
    `;
    document.documentElement.appendChild(style);
  }

  // per-page state
  if (!window.__MTF__) window.__MTF__ = { query: "", matches: [], index: -1 };

  function clearHighlights() {
    const nodes = document.querySelectorAll("." + HIGHLIGHT);
    nodes.forEach(span => {
      const p = span.parentNode;
      while (span.firstChild) p.insertBefore(span.firstChild, span);
      p.removeChild(span);
      p.normalize?.();
    });
    window.__MTF__.matches = [];
    window.__MTF__.index = -1;
  }

  function textNodes(root) {
    const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT, {
      acceptNode(n) {
        const pe = n.parentElement;
        if (!pe) return NodeFilter.FILTER_REJECT;
        const tag = pe.tagName;
        if (/^(SCRIPT|STYLE|NOSCRIPT|IFRAME|OBJECT|SVG|CANVAS|CODE|PRE)$/i.test(tag)) return NodeFilter.FILTER_REJECT;
        const cs = getComputedStyle(pe);
        if (cs.display === "none" || cs.visibility === "hidden") return NodeFilter.FILTER_REJECT;
        if (!n.nodeValue || !n.nodeValue.trim()) return NodeFilter.FILTER_REJECT;
        return NodeFilter.FILTER_ACCEPT;
      }
    });
    const out = [];
    let x;
    while ((x = walker.nextNode())) out.push(x);
    return out;
  }

  function highlightQuery(q) {
    clearHighlights();
    if (!q) return 0;

    // escape literal; case-insensitive
    const re = new RegExp(q.replace(/[.*+?^${}()|[\]\\]/g, "\\$&"), "gi");
    const nodes = textNodes(document.body);
    const matches = [];

    for (const tn of nodes) {
      const text = tn.nodeValue;
      if (!re.test(text)) continue;
      re.lastIndex = 0;

      const frag = document.createDocumentFragment();
      let last = 0, m;
      while ((m = re.exec(text)) !== null) {
        const i = m.index;
        const s = m[0];
        if (i > last) frag.appendChild(document.createTextNode(text.slice(last, i)));
        const mark = document.createElement("mark");
        mark.className = HIGHLIGHT;
        mark.textContent = s;
        frag.appendChild(mark);
        matches.push(mark);
        last = i + s.length;
        if (re.lastIndex === i) re.lastIndex++; // safety
      }
      if (last < text.length) frag.appendChild(document.createTextNode(text.slice(last)));
      tn.parentNode.replaceChild(frag, tn);
    }

    window.__MTF__.query = q;
    window.__MTF__.matches = matches;
    window.__MTF__.index = matches.length ? 0 : -1;

    if (matches.length) {
      setCurrent(0);
      scrollCurrent();
    }
    return matches.length;
  }

  function setCurrent(i) {
    const st = window.__MTF__;
    st.matches.forEach(el => el.classList.remove(CURRENT));
    if (st.matches[i]) st.matches[i].classList.add(CURRENT);
    st.index = i;
  }

  function scrollCurrent() {
    const st = window.__MTF__;
    const el = st.matches[st.index];
    if (!el) return;
    // open <details> ancestors so scroll works
    let p = el.parentElement;
    while (p) { if (p.tagName === "DETAILS") p.open = true; p = p.parentElement; }
    el.scrollIntoView({ block: "center", inline: "nearest", behavior: "smooth" });
  }

  function next() {
    const st = window.__MTF__;
    const n = st.matches.length;
    if (!n) return { index: -1, total: 0 };
    st.index = (st.index + 1) % n;
    setCurrent(st.index);
    scrollCurrent();
    return { index: st.index, total: n };
  }

  function prev() {
    const st = window.__MTF__;
    const n = st.matches.length;
    if (!n) return { index: -1, total: 0 };
    st.index = (st.index - 1 + n) % n;
    setCurrent(st.index);
    scrollCurrent();
    return { index: st.index, total: n };
  }

  chrome.runtime.onMessage.addListener((msg, _sender, send) => {
    try {
      if (msg?.type === "MTF_SEARCH") return send({ count: highlightQuery(msg.query || "") });
      if (msg?.type === "MTF_NEXT")   return send(next());
      if (msg?.type === "MTF_PREV")   return send(prev());
      if (msg?.type === "MTF_CLEAR")  { clearHighlights(); return send({ ok: true }); }
      if (msg?.type === "MTF_INFO")   {
        const st = window.__MTF__;
        return send({ query: st.query, count: st.matches.length, index: st.index });
      }
    } catch (e) {
      send({ ok: false, error: String(e) });
    }
  });
})();