// Popup controller: runs only in the popup.
const $ = s => document.querySelector(s);
const queryInput = $("#query");
const btnSearch  = $("#search");
const btnNext    = $("#next");
const btnPrev    = $("#prev");
const btnClear   = $("#clear");
const summary    = $("#summary");
const tabList    = $("#tabList");

let state = {
  query: "",
  tabs: [],           // [{tabId,title,count}]
  currentTabIdx: -1   // index into state.tabs (first tab with matches)
};

function isInjectable(url) {
  return /^https?:|^file:|^ftp:/i.test(url || "");
}

async function tabsInCurrentWindow() {
  return chrome.tabs.query({ currentWindow: true });
}

async function ensureContent(tabId) {
  try { await chrome.tabs.sendMessage(tabId, { type: "MTF_INFO" }); }
  catch { await chrome.scripting.executeScript({ target: { tabId }, files: ["content.js"] }); }
}

function render() {
  tabList.innerHTML = "";
  let total = 0;

  state.tabs.forEach((t, i) => {
    total += t.count || 0;
    const li = document.createElement("li");
    if (i === state.currentTabIdx) li.classList.add("active");
    li.innerHTML = `<span class="count">${t.count || 0}</span><span class="title">${t.title || "(untitled)"}</span>`;
    tabList.appendChild(li);
  });

  summary.textContent = state.query
    ? `“${state.query}” — ${total} matches in ${state.tabs.length} tabs`
    : "No search yet.";

  const has = total > 0;
  btnNext.disabled = btnPrev.disabled = btnClear.disabled = !has;
}

async function doSearch() {
  const q = queryInput.value.trim();
  if (!q) return;

  state.query = q;
  state.tabs = [];
  state.currentTabIdx = -1;

  const tabs = await tabsInCurrentWindow();

  // Process all tabs in parallel for speed.
  await Promise.all(tabs.map(async (tab) => {
    const rec = { tabId: tab.id, title: tab.title, count: 0 };
    if (!isInjectable(tab.url)) { state.tabs.push(rec); return; }

    try {
      await ensureContent(tab.id);
      const r = await chrome.tabs.sendMessage(tab.id, { type: "MTF_SEARCH", query: q });
      rec.count = r?.count || 0;
    } catch (_) {
      rec.count = 0;
    }
    state.tabs.push(rec);
  }));

  // pick first tab with matches
  state.currentTabIdx = state.tabs.findIndex(t => (t.count || 0) > 0);

  render();
}

async function step(direction) {
  if (state.currentTabIdx < 0 || state.tabs.length === 0) return;

  const idx = state.currentTabIdx;
  const tab = state.tabs[idx];
  if (!tab || !tab.tabId) return;

  try {
    const res = await chrome.tabs.sendMessage(
      tab.tabId,
      { type: direction > 0 ? "MTF_NEXT" : "MTF_PREV" }
    );

    if (!res || res.total <= 0) return;

    const wrapped =
      (direction > 0 && res.index === 0) ||
      (direction < 0 && res.index === res.total - 1);

    if (wrapped) {
      // find next tab with matches
      const n = state.tabs.length;
      for (let i = 1; i <= n; i++) {
        const j = (idx + (direction > 0 ? i : -i) + n) % n;
        if ((state.tabs[j].count || 0) > 0) {
          state.currentTabIdx = j;
          const nextTab = state.tabs[j];
          // activate next tab but keep popup open (no window focus)
          await chrome.tabs.update(nextTab.tabId, { active: true });
          // show first/last match
          await chrome.tabs.sendMessage(
            nextTab.tabId,
            { type: direction > 0 ? "MTF_NEXT" : "MTF_PREV" }
          );
          break;
        }
      }
      render();
    }
  } catch (err) {
    console.error("step() error:", err);
  }
}

async function clearAll() {
  const tabs = await tabsInCurrentWindow();
  await Promise.all(tabs.map(async (t) => {
    try { await chrome.tabs.sendMessage(t.id, { type: "MTF_CLEAR" }); } catch {}
  }));
  state = { query: "", tabs: [], currentTabIdx: -1 };
  render();
}

/* UI */
btnSearch.addEventListener("click", doSearch);
queryInput.addEventListener("keydown", e => { if (e.key === "Enter") doSearch(); });
btnNext.addEventListener("click", () => step(1));
btnPrev.addEventListener("click", () => step(-1));
btnClear.addEventListener("click", clearAll);

/* disable Search if empty */
btnSearch.disabled = true;
queryInput.addEventListener("input", () => { btnSearch.disabled = queryInput.value.trim().length === 0; });

render();