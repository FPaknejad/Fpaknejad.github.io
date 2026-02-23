async function getActiveTab() {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });
  return tab;
}

function fillSelect(select, languages) {
  select.textContent = "";
  const none = document.createElement("option");
  none.value = "";
  none.textContent = "— None —";
  select.appendChild(none);

  for (const lang of languages) {
    const opt = document.createElement("option");
    opt.value = lang.id; // id is stable key we get from content.js
    opt.textContent = `${lang.label} (${lang.code || "?"})`;
    select.appendChild(opt);
  }
}

async function init() {
  const tab = await getActiveTab();

  // Ask content script for available subtitle languages
  const languages = await chrome.tabs.sendMessage(tab.id, { type: "GET_LANGUAGES" }).catch(() => null) || [];

  const primarySel   = document.getElementById("primary");
  const secondarySel = document.getElementById("secondary");
  fillSelect(primarySel, languages);
  fillSelect(secondarySel, languages);

  // Load previous selections
  const { dualSubsSelections = {} } = await chrome.storage.local.get("dualSubsSelections");
  if (dualSubsSelections.primary)   primarySel.value   = dualSubsSelections.primary;
  if (dualSubsSelections.secondary) secondarySel.value = dualSubsSelections.secondary;

  document.getElementById("apply").addEventListener("click", async () => {
    const primary = primarySel.value || null;
    const secondary = secondarySel.value || null;

    await chrome.storage.local.set({ dualSubsSelections: { primary, secondary } });
    const ok = await chrome.tabs.sendMessage(tab.id, {
      type: "SET_DUAL_SUBS",
      primary,
      secondary
    }).catch(() => false);

    document.getElementById("status").textContent = ok ? "Applied." : "Could not apply on this page.";
  });
}

document.addEventListener("DOMContentLoaded", init);