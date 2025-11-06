(() => {
  // --- Simple parsers --------------------------------------------------------
  function parseVTT(text) {
    // Basic WebVTT parser: returns [{start,end,text}]
    const cues = [];
    const lines = text.replace(/\r/g, "").split("\n");
    let i = 0;

    // skip "WEBVTT" header if present
    if (lines[i] && /^WEBVTT/i.test(lines[i])) i++;

    while (i < lines.length) {
      // Skip optional cue id line
      if (lines[i] && !lines[i].includes("-->")) i++;

      if (i >= lines.length) break;
      const ts = lines[i++];
      if (!/-->/.test(ts)) continue;

      const [startS, endS] = ts.split("-->").map(s => s.trim());
      let textLines = [];
      while (i < lines.length && lines[i].trim() !== "") textLines.push(lines[i++]);
      // skip blank line
      while (i < lines.length && lines[i].trim() === "") i++;

      const textBlock = textLines.join("\n")
        // strip basic tags frequently seen in VTT
        .replace(/<[^>]+>/g, "")
        .trim();

      cues.push({
        start: toSeconds(startS),
        end: toSeconds(endS),
        text: textBlock
      });
    }
    return cues;
  }

  function parseSRT(text) {
    const entries = text.replace(/\r/g, "").trim().split(/\n\n+/);
    const cues = [];
    for (const block of entries) {
      const lines = block.split("\n");
      // first line may be number
      let idx = 0;
      if (/^\d+$/.test(lines[0])) idx = 1;

      const ts = lines[idx++];
      const m = ts && ts.match(/(\d+:\d{2}:\d{2}[,\.]\d{3})\s*-->\s*(\d+:\d{2}:\d{2}[,\.]\d{3})/);
      if (!m) continue;

      const textLines = lines.slice(idx);
      const textBlock = textLines.join("\n").replace(/<[^>]+>/g, "").trim();
      cues.push({
        start: toSeconds(m[1].replace(",", ".")),
        end:   toSeconds(m[2].replace(",", ".")),
        text:  textBlock
      });
    }
    return cues;
  }

  function toSeconds(hms) {
    // "00:01:02.345" or "00:01:02,345"
    const [h, m, s] = hms.replace(",", ".").split(":").map(parseFloat);
    return (h * 3600) + (m * 60) + s;
  }

  // --- Registry & state ------------------------------------------------------
  const STATE = {
    // key: languageId => { id, code, label, cues:[{start,end,text}], sourceUrl }
    tracks: new Map(),
    // associate videos with overlay
    overlays: new WeakMap(),
    // remember selected ids
    selected: { primary: null, secondary: null }
  };

  // Utility: language id from url or code+label
  function makeLangId({ code, label, url }) {
    const base = code || label || new URL(url, location.href).pathname.split("/").pop();
    return (base || "lang").toLowerCase() + ":" + (url ? hash(url) : Math.random().toString(36).slice(2));
  }
  function hash(s) {
    let h = 0;
    for (let i = 0; i < s.length; i++) h = (h * 31 + s.charCodeAt(i)) >>> 0;
    return h.toString(36);
  }

  // --- Capture subtitle payloads by patching fetch & XHR ---------------------
  // Many sites (including anime mirrors) load subtitles with fetch/XHR.
  // We wrap them and sniff responses that look like subtitles.

  const isProbablySubtitleUrl = (url) => {
    try {
      const u = new URL(url, location.href);
      const p = u.pathname.toLowerCase();
      return (
        p.endsWith(".vtt") ||
        p.endsWith(".srt") ||
        p.endsWith(".ass") ||
        p.includes("/sub") || p.includes("subtitle") || p.includes("captions")
      );
    } catch {
      return false;
    }
  };

  function registerSubFromText({ url, mime, text }) {
    let cues = [];
    let label = "";
    let code = "";

    // Try to infer language from query/path, e.g. ?lang=en or .../en.vtt
    const u = new URL(url, location.href);
    code = u.searchParams.get("lang") || u.searchParams.get("locale") || "";
    if (!code) {
      const m = u.pathname.match(/[\/\._-]([a-z]{2,3})(?:-[A-Z]{2})?\.(vtt|srt|ass)$/i);
      if (m) code = m[1];
    }
    label = code ? code.toUpperCase() : "Unknown";

    // Parse based on file type
    if (/\.vtt($|\?)/i.test(url) || /vtt/i.test(mime)) {
      cues = parseVTT(text);
    } else if (/\.srt($|\?)/i.test(url) || /srt/i.test(mime)) {
      cues = parseSRT(text);
    } else if (/\.ass($|\?)/i.test(url) || /ass/i.test(mime)) {
      // Minimal: strip markup, no positioning
      const lines = text.replace(/\r/g, "").split("\n");
      const dialog = lines.filter(l => l.startsWith("Dialogue:"));
      // ASS timestamps are "h:mm:ss.cs"
      cues = dialog.map(l => {
        const parts = l.split(",");
        const start = parts[1]?.trim() || "0:00:00.00";
        const end   = parts[2]?.trim() || "0:00:00.00";
        const raw   = parts.slice(9).join(",").replace(/{[^}]+}/g, "").trim();
        return { start: toSecondsFix(start), end: toSecondsFix(end), text: raw };
      });
      function toSecondsFix(t) {
        const [h, m, s] = t.replace(",", ".").split(":");
        return (+h) * 3600 + (+m) * 60 + parseFloat(s);
      }
      label = label + " (ASS)";
    } else if (/json/i.test(mime)) {
      // Some players use JSON arrays; try to parse simple structures
      try {
        const data = JSON.parse(text);
        // Heuristics: look for [{start,end,text}] or [{from,to,content}]
        if (Array.isArray(data)) {
          cues = data.map(it => ({
            start: it.start ?? it.from ?? it.t ?? 0,
            end:   it.end   ?? it.to   ?? (it.t ? it.t + (it.d || 2) : 0),
            text:  (it.text ?? it.content ?? "").toString().replace(/<[^>]+>/g, "")
          })).filter(c => c.text);
        }
      } catch {}
    }

    if (cues.length) {
      const id = makeLangId({ code, label, url });
      STATE.tracks.set(id, { id, code, label, cues, sourceUrl: url });
      // Notify popup (if open)
      chrome.runtime.sendMessage?.({ type: "DUAL_SUBS_LANGS_UPDATE" }).catch(() => {});
    }
  }

  // Patch fetch
  const origFetch = window.fetch;
  window.fetch = async function patchedFetch(input, init) {
    const res = await origFetch(input, init);
    try {
      const url = typeof input === "string" ? input : input.url;
      if (isProbablySubtitleUrl(url)) {
        const clone = res.clone();
        const ct = clone.headers.get("content-type") || "";
        const text = await clone.text();
        registerSubFromText({ url, mime: ct, text });
      }
    } catch {}
    return res;
  };

  // Patch XHR
  const origOpen = XMLHttpRequest.prototype.open;
  const origSend = XMLHttpRequest.prototype.send;
  XMLHttpRequest.prototype.open = function(method, url, ...rest) {
    this.__dualSubsUrl = url;
    return origOpen.call(this, method, url, ...rest);
  };
  XMLHttpRequest.prototype.send = function(...args) {
    this.addEventListener("load", function() {
      try {
        const url = this.__dualSubsUrl;
        if (!url || !isProbablySubtitleUrl(url)) return;
        const ct = this.getResponseHeader("content-type") || "";
        const text = (typeof this.response === "string") ? this.response : this.responseText;
        if (text) registerSubFromText({ url, mime: ct, text });
      } catch {}
    });
    return origSend.apply(this, args);
  };

  // Also catch already-present <track> files (rare on such sites, but cheap to support)
  function scanTracksInDom() {
    document.querySelectorAll("video track[src]").forEach(async tr => {
      try {
        const url = new URL(tr.getAttribute("src"), location.href).href;
        const res = await fetch(url);
        const ct  = res.headers.get("content-type") || "";
        const txt = await res.text();
        registerSubFromText({ url, mime: ct, text: txt });
      } catch {}
    });
  }

  // --- Overlay render loop per <video> ---------------------------------------
  function ensureOverlayFor(video) {
    if (STATE.overlays.has(video)) return STATE.overlays.get(video);

    // Create overlay container
    const overlay = document.createElement("div");
    overlay.className = "dual-subs-overlay dual-subs-bottom";
    // Positioning context
    const parent = video.parentElement || document.body;
    const oldPos = getComputedStyle(parent).position;
    if (!/relative|absolute|fixed|sticky/i.test(oldPos)) {
      parent.style.position = "relative";
    }
    parent.appendChild(overlay);

    const primaryEl = document.createElement("div");
    const secondaryEl = document.createElement("div");
    primaryEl.className = "dual-subs-line primary";
    secondaryEl.className = "dual-subs-line secondary";
    overlay.appendChild(secondaryEl);
    overlay.appendChild(primaryEl);

    const ctrl = {
      overlay, primaryEl, secondaryEl,
      current: { primary: null, secondary: null },
      raf: null
    };

    function findCue(cues, t) {
      // binary search (cues are ordered)
      let lo = 0, hi = cues.length - 1, match = null;
      while (lo <= hi) {
        const mid = (lo + hi) >> 1;
        const c = cues[mid];
        if (t < c.start) hi = mid - 1;
        else if (t > c.end) lo = mid + 1;
        else { match = c; break; }
      }
      return match;
    }

    function tick() {
      const t = video.currentTime || 0;

      const p = STATE.selected.primary   ? STATE.tracks.get(STATE.selected.primary)   : null;
      const s = STATE.selected.secondary ? STATE.tracks.get(STATE.selected.secondary) : null;

      // Update DOM only if changed
      if (p?.cues) {
        const cue = findCue(p.cues, t);
        const text = cue ? cue.text : "";
        if (ctrl.current.primary !== text) {
          ctrl.current.primary = text;
          ctrl.primaryEl.textContent = text;
        }
      } else if (ctrl.current.primary !== "") {
        ctrl.current.primary = "";
        ctrl.primaryEl.textContent = "";
      }

      if (s?.cues) {
        const cue = findCue(s.cues, t);
        const text = cue ? cue.text : "";
        if (ctrl.current.secondary !== text) {
          ctrl.current.secondary = text;
          ctrl.secondaryEl.textContent = text;
        }
      } else if (ctrl.current.secondary !== "") {
        ctrl.current.secondary = "";
        ctrl.secondaryEl.textContent = "";
      }

      ctrl.raf = requestAnimationFrame(tick);
    }

    ctrl.raf = requestAnimationFrame(tick);
    STATE.overlays.set(video, ctrl);
    return ctrl;
  }

  function applySelectionsToAllVideos() {
    const videos = document.querySelectorAll("video");
    videos.forEach(v => ensureOverlayFor(v));
  }

  // Observe dynamically-added videos
  const mo = new MutationObserver(() => {
    applySelectionsToAllVideos();
  });
  mo.observe(document.documentElement, { childList: true, subtree: true });

  // Initial scans
  scanTracksInDom();
  applySelectionsToAllVideos();

  // --- Messaging with popup ---------------------------------------------------
  chrome.runtime.onMessage.addListener((msg, _sender, sendResponse) => {
    if (msg?.type === "GET_LANGUAGES") {
      const list = Array.from(STATE.tracks.values()).map(t => ({
        id: t.id, code: t.code, label: t.label, url: t.sourceUrl
      }));
      sendResponse(list);
      return true;
    }

    if (msg?.type === "SET_DUAL_SUBS") {
      STATE.selected.primary = msg.primary || null;
      STATE.selected.secondary = msg.secondary || null;
      // render on all videos
      applySelectionsToAllVideos();
      sendResponse(true);
      return true;
    }
  });

  // Also restore saved selections (if popup set them earlier)
  chrome.storage?.local.get("dualSubsSelections").then(({ dualSubsSelections }) => {
    if (dualSubsSelections) {
      STATE.selected = { ...STATE.selected, ...dualSubsSelections };
      applySelectionsToAllVideos();
    }
  }).catch(() => {});
})();