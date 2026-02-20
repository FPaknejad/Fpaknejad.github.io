import { lookupGermanWord } from "./wiktionaryClient.js";

const form = document.getElementById("lookup-form");
const input = document.getElementById("word-input");
const button = document.getElementById("lookup-btn");
const statusEl = document.getElementById("status");
const resultEl = document.getElementById("result");

let inflightWord = null;
let requestToken = 0;
let lastCompletedWord = null;

function normalizeLookupWord(value) {
  return String(value || "")
    .replace(/\s+/g, " ")
    .trim();
}

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/\"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

function unknownBadge(value) {
  if (value !== "Unknown") {
    return escapeHtml(value);
  }
  return `${value} <span class="badge-unknown">missing</span>`;
}

function getCaseShortLabel(caseGovernance) {
  if (caseGovernance === "Akkusativ") {
    return "Akk.";
  }
  if (caseGovernance === "Dativ") {
    return "Dat.";
  }
  if (caseGovernance === "Akkusativ+Dativ") {
    return "Akk.+Dat.";
  }
  return "";
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

function renderPerfectValue(verbInfo) {
  const perfectIch = verbInfo?.perfectConjugation?.ich;
  if (perfectIch && perfectIch !== "Unknown") {
    return escapeHtml(perfectIch);
  }

  const participle = verbInfo?.pastParticiple;
  if (!participle || participle === "Unknown") {
    return unknownBadge("Unknown");
  }

  const auxiliaries = Array.isArray(verbInfo?.auxiliaryVerbs)
    ? verbInfo.auxiliaryVerbs.filter((value) => value && value !== "Unknown")
    : [];
  if (auxiliaries.length === 0) {
    return escapeHtml(participle);
  }

  return escapeHtml(`ich ${auxiliaries[0]} ${participle}`);
}

function renderIdle(message) {
  statusEl.textContent = message;
  resultEl.innerHTML = "";
}

function renderLoading(word) {
  statusEl.textContent = `Looking up \"${word}\"...`;
  resultEl.innerHTML = "";
}

function renderError(message) {
  statusEl.textContent = "Error";
  resultEl.innerHTML = `<div class="card"><div class="row">${escapeHtml(message)}</div></div>`;
}

function renderNotFound(word) {
  statusEl.textContent = "No entry found";
  resultEl.innerHTML = `<div class="card"><div class="row">No Wiktionary result found for \"${escapeHtml(word)}\".</div></div>`;
}

function renderSuccess(result) {
  statusEl.textContent = "Done";

  const translations = Array.isArray(result?.translations)
    ? result.translations
    : [];

  const translationsHtml =
    translations.length > 0
      ? `<div class="row">${translations
          .map((value) => escapeHtml(value))
          .join(", ")}</div>`
      : `<div class="row">No English translations extracted.</div>`;

  const caseShort = getCaseShortLabel(result?.verbInfo?.caseGovernance);
  const caseTag =
    result.partOfSpeech === "Verb" && caseShort
      ? ` <span class="case-tag">(${escapeHtml(caseShort)})</span>`
      : "";

  const headwordLine = `${escapeHtml(result.title)} <span class="pos-inline">(${escapeHtml(
    result.partOfSpeech || "Unknown"
  )})</span>${caseTag}`;

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
          result.verbInfo.caseGovernance &&
          result.verbInfo.caseGovernance !== "Unknown"
            ? `<div class="row"><span class="label">Case governance:</span> ${escapeHtml(result.verbInfo.caseGovernance)}</div>`
            : ""
        }
        ${
          Array.isArray(result.verbInfo.infinitiveForms) &&
          result.verbInfo.infinitiveForms.length > 0
            ? `<div class="row"><span class="label">Forms:</span> ${result.verbInfo.infinitiveForms
                .map((value) => escapeHtml(value))
                .join(" | ")}</div>`
            : ""
        }
        <table class="conj-table">
          <thead>
            <tr>
              <th>Praesens</th>
              <th>Praeteritum</th>
              <th>Partizip II</th>
            </tr>
          </thead>
          <tbody>
            <tr>
              <td>${renderHoverValue(result.verbInfo.present, result.verbInfo.presentConjugation)}</td>
              <td>${renderHoverValue(result.verbInfo.preterite, result.verbInfo.preteriteConjugation)}</td>
              <td>${renderPastParticipleValue(result.verbInfo)}</td>
            </tr>
            <tr>
              <th>Perfekt</th>
              <td colspan="2">${renderPerfectValue(result.verbInfo)}</td>
            </tr>
          </tbody>
        </table>
      </div>
    `
    : "";

  const notesHtml =
    result.notes && result.notes.length
      ? `<div class="card"><h2>Notes</h2><ul class="notes">${result.notes
          .map((note) => `<li>${escapeHtml(note)}</li>`)
          .join("")}</ul></div>`
      : "";

  const derivedVariantStates = Array.isArray(result.derivedVariantStates)
    ? result.derivedVariantStates
    : Array.isArray(result.validDerivedVariants)
      ? result.validDerivedVariants.map((term) => ({ term, clickable: true }))
      : [];
  const derivedHtml =
    derivedVariantStates.length > 0
      ? `
      <div class="card">
        <h2>Derived terms</h2>
        <div class="derived-list">
          ${derivedVariantStates
            .map((item) => {
              const term = escapeHtml(item.term);
              if (item.clickable) {
                return `<button class="derived-btn" type="button" data-variant="${term}">${term}</button>`;
              }
              return `<button class="derived-btn disabled" type="button" disabled title="No English translation found">${term}</button>`;
            })
            .join("")}
        </div>
      </div>
    `
      : "";

  resultEl.innerHTML = `
    <div class="card">
      <div class="header-row">
        <h2 class="headword-line">${headwordLine}</h2>
        <button class="speak-btn" type="button" data-speak="${escapeHtml(result.title)}" aria-label="Speak word">🔊</button>
      </div>
      ${translationsHtml}
    </div>
    ${nounHtml}
    ${verbHtml}
    ${derivedHtml}
    ${notesHtml}
  `;
}

function speakWord(word) {
  const normalized = normalizeLookupWord(word);
  if (!normalized) {
    return;
  }

  if (!("speechSynthesis" in window)) {
    statusEl.textContent = "Text-to-speech is unavailable in this browser.";
    return;
  }

  window.speechSynthesis.cancel();
  const utterance = new SpeechSynthesisUtterance(normalized);
  utterance.lang = "de-DE";
  utterance.rate = 0.95;
  utterance.onerror = () => {
    statusEl.textContent = "Could not play pronunciation.";
  };
  window.speechSynthesis.speak(utterance);
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
  const normalized = normalizeLookupWord(word);
  if (!normalized) {
    renderIdle("Enter a German word to translate.");
    lastCompletedWord = null;
    return;
  }

  if (inflightWord && inflightWord.toLowerCase() === normalized.toLowerCase()) {
    return;
  }

  if (
    lastCompletedWord &&
    lastCompletedWord.toLowerCase() === normalized.toLowerCase()
  ) {
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
      lastCompletedWord = normalized;
    } else {
      renderSuccess(response.result);
      lastCompletedWord = normalized;
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

input.addEventListener("input", () => {
  lastCompletedWord = null;
});

form.addEventListener("submit", (event) => {
  event.preventDefault();
  runLookup(input.value);
});

resultEl.addEventListener("click", (event) => {
  const trigger = event.target.closest("[data-speak], [data-variant]");
  if (!trigger) {
    return;
  }
  const speakValue = trigger.getAttribute("data-speak");
  if (speakValue) {
    speakWord(speakValue);
    return;
  }

  const variant = trigger.getAttribute("data-variant");
  if (variant) {
    input.value = variant;
    runLookup(variant);
  }
});

window.addEventListener("DOMContentLoaded", async () => {
  renderIdle("Select a German word on a page, or type one above.");
  input.focus();
  const selectedWord = await getHighlightedWord();
  if (selectedWord) {
    input.value = selectedWord;
    input.select();
    runLookup(selectedWord);
  }
});
