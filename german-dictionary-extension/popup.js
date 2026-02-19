import { lookupGermanWord } from "./wiktionaryClient.js";

const form = document.getElementById("lookup-form");
const input = document.getElementById("word-input");
const button = document.getElementById("lookup-btn");
const statusEl = document.getElementById("status");
const resultEl = document.getElementById("result");

let debounceTimer = null;
let inflightWord = null;
let requestToken = 0;

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

function unknownBadge(value) {
  if (value !== "Unknown") {
    return escapeHtml(value);
  }
  return `${value} <span class="badge-unknown">missing</span>`;
}

function buildConjugationTooltip(conjugation) {
  if (!conjugation) {
    return "";
  }

  const rows = Object.entries(conjugation)
    .map(
      ([person, value]) =>
        `<div>${escapeHtml(person)}: ${escapeHtml(value || "Unknown")}</div>`
    );

  return `<span class="tooltip">${rows.join("")}</span>`;
}

function renderHoverValue(value, conjugation) {
  const base = unknownBadge(value);
  const tooltip = buildConjugationTooltip(conjugation);
  if (!tooltip) {
    return base;
  }
  return `<span class="hover-target">${base}${tooltip}</span>`;
}

function renderPastParticipleValue(verbInfo) {
  const base = verbInfo.pastParticiple;
  if (base === "Unknown") {
    return unknownBadge(base);
  }

  const auxiliaries = Array.isArray(verbInfo.auxiliaryVerbs)
    ? verbInfo.auxiliaryVerbs.filter((value) => value && value !== "Unknown")
    : [];
  const suffix = auxiliaries.length ? ` (${auxiliaries.join(" / ")})` : "";
  return renderHoverValue(`${base}${suffix}`, verbInfo.perfectConjugation);
}

function renderIdle(message) {
  statusEl.textContent = message;
  resultEl.innerHTML = "";
}

function renderLoading(word) {
  statusEl.textContent = `Looking up "${word}"...`;
  resultEl.innerHTML = "";
}

function renderError(message) {
  statusEl.textContent = "Error";
  resultEl.innerHTML = `<div class="card"><div class="row">${escapeHtml(message)}</div></div>`;
}

function renderNotFound(word) {
  statusEl.textContent = "No entry found";
  resultEl.innerHTML = `<div class="card"><div class="row">No Wiktionary result found for "${escapeHtml(word)}".</div></div>`;
}

function renderSuccess(result) {
  statusEl.textContent = "Done";

  const translationsHtml =
    result.translations.length > 0
      ? `<ul class="translations">${result.translations
          .map((value) => `<li>${escapeHtml(value)}</li>`)
          .join("")}</ul>`
      : `<div class="row">No English translations extracted.</div>`;

  const nounHtml = result.nounInfo
    ? `
      <div class="card">
        <h2>Noun</h2>
        <div class="row"><span class="label">Article:</span> ${unknownBadge(result.nounInfo.article)}</div>
        <div class="row"><span class="label">Plural:</span> ${unknownBadge(result.nounInfo.plural)}</div>
      </div>
    `
    : "";

  const verbHtml = result.verbInfo
    ? `
      <div class="card">
        <h2>Verb</h2>
        ${
          Array.isArray(result.verbInfo.infinitiveForms) &&
          result.verbInfo.infinitiveForms.length > 0
            ? `<div class="row"><span class="label">Forms:</span> ${result.verbInfo.infinitiveForms
                .map((value) => escapeHtml(value))
                .join(" | ")}</div>`
            : ""
        }
        <div class="row"><span class="label">Present:</span> ${renderHoverValue(result.verbInfo.present, result.verbInfo.presentConjugation)}</div>
        <div class="row"><span class="label">Past tense:</span> ${renderHoverValue(result.verbInfo.preterite, result.verbInfo.preteriteConjugation)}</div>
        <div class="row"><span class="label">Past participle:</span> ${renderPastParticipleValue(result.verbInfo)}</div>
        <div class="row"><span class="label">Case governance:</span> ${unknownBadge(result.verbInfo.caseGovernance)}</div>
        ${
          Array.isArray(result.verbInfo.caseGovernanceDetails) &&
          result.verbInfo.caseGovernanceDetails.length > 0
            ? `<div class="row"><span class="label">Case details:</span></div>
               <ul class="translations">${result.verbInfo.caseGovernanceDetails
                 .map((detail) => `<li>${escapeHtml(detail)}</li>`)
                 .join("")}</ul>`
            : ""
        }
      </div>
    `
    : "";

  const notesHtml =
    result.notes && result.notes.length
      ? `<div class="card"><h2>Notes</h2><ul class="notes">${result.notes
          .map((note) => `<li>${escapeHtml(note)}</li>`)
          .join("")}</ul></div>`
      : "";

  resultEl.innerHTML = `
    <div class="card">
      <h2>${escapeHtml(result.title)}</h2>
      <div class="row"><span class="label">Part of speech:</span> ${escapeHtml(result.partOfSpeech)}</div>
      ${translationsHtml}
    </div>
    ${nounHtml}
    ${verbHtml}
    ${notesHtml}
  `;
}

async function getHighlightedWord() {
  try {
    const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });
    if (!tab?.id) {
      return "";
    }

    const injection = await chrome.scripting.executeScript({
      target: { tabId: tab.id },
      func: () => window.getSelection()?.toString() || ""
    });

    const selected = (injection?.[0]?.result || "").trim();
    return selected;
  } catch {
    return "";
  }
}

async function runLookup(word) {
  const normalized = (word || "").trim();
  if (!normalized) {
    renderIdle("Enter a German word to translate.");
    return;
  }

  if (inflightWord && inflightWord.toLowerCase() === normalized.toLowerCase()) {
    return;
  }

  requestToken += 1;
  const token = requestToken;
  inflightWord = normalized;
  button.disabled = true;
  renderLoading(normalized);

  try {
    const response = await lookupGermanWord(normalized);
    if (token !== requestToken) {
      return;
    }

    if (response.status === "not_found") {
      renderNotFound(normalized);
    } else {
      renderSuccess(response.result);
    }
  } catch (error) {
    if (token !== requestToken) {
      return;
    }
    renderError(error.message || "Lookup failed.");
  } finally {
    if (token === requestToken) {
      inflightWord = null;
      button.disabled = false;
    }
  }
}

function scheduleLookup(word) {
  clearTimeout(debounceTimer);
  debounceTimer = setTimeout(() => {
    runLookup(word);
  }, 250);
}

form.addEventListener("submit", (event) => {
  event.preventDefault();
  scheduleLookup(input.value);
});

window.addEventListener("DOMContentLoaded", async () => {
  renderIdle("Select a German word on a page, or type one above.");
  const selectedWord = await getHighlightedWord();
  if (selectedWord) {
    input.value = selectedWord;
    runLookup(selectedWord);
  }
});
