import { lookupGermanWord } from "./wiktionaryClient.js";
import { highlightExampleText } from "./exampleHighlight.js";

const form = document.getElementById("lookup-form");
const input = document.getElementById("word-input");
const button = document.getElementById("lookup-btn");
const exportBtn = document.getElementById("export-btn");
const statusEl = document.getElementById("status");
const resultEl = document.getElementById("result");

let inflightWord = null;
let requestToken = 0;
let lastCompletedWord = null;
let exportInFlight = false;

const HISTORY_KEY = "lookupHistoryV1";

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

function emptyIfUnknown(value) {
  if (value == null) {
    return "";
  }
  const normalized = String(value).trim();
  return normalized === "Unknown" ? "" : normalized;
}

function normalizeForKey(value) {
  return String(value || "")
    .toLocaleLowerCase("de-DE")
    .replace(/\s+/g, " ")
    .trim();
}

function hasStorageApi() {
  return Boolean(chrome?.storage?.local);
}

function hasDownloadsApi() {
  return Boolean(chrome?.downloads?.download);
}

function storageGet(key) {
  return new Promise((resolve, reject) => {
    if (!hasStorageApi()) {
      reject(new Error("Storage-API ist nicht verfuegbar."));
      return;
    }
    chrome.storage.local.get([key], (items) => {
      if (chrome.runtime?.lastError) {
        reject(new Error(chrome.runtime.lastError.message));
        return;
      }
      resolve(items?.[key]);
    });
  });
}

function storageSet(values) {
  return new Promise((resolve, reject) => {
    if (!hasStorageApi()) {
      reject(new Error("Storage-API ist nicht verfuegbar."));
      return;
    }
    chrome.storage.local.set(values, () => {
      if (chrome.runtime?.lastError) {
        reject(new Error(chrome.runtime.lastError.message));
        return;
      }
      resolve();
    });
  });
}

async function readLookupHistory() {
  const value = await storageGet(HISTORY_KEY);
  return Array.isArray(value) ? value : [];
}

async function writeLookupHistory(entries) {
  await storageSet({ [HISTORY_KEY]: Array.isArray(entries) ? entries : [] });
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
  if (caseGovernance === "Genitiv") {
    return "Gen.";
  }
  return "";
}

function localizePartOfSpeech(partOfSpeech) {
  if (partOfSpeech === "Noun") {
    return "Substantiv";
  }
  if (partOfSpeech === "Verb") {
    return "Verb";
  }
  if (partOfSpeech === "Adjective") {
    return "Adjektiv";
  }
  return "Unbekannt";
}

function derivePluralEnding(singular, plural) {
  const base = String(singular || "").trim();
  const pl = String(plural || "").trim();
  if (!base || !pl || pl === "Unknown") {
    return "-";
  }
  if (pl === base) {
    return "-";
  }

  const normalizeUmlaut = (value) =>
    value
      .replace(/ä/g, "a")
      .replace(/ö/g, "o")
      .replace(/ü/g, "u")
      .replace(/Ä/g, "A")
      .replace(/Ö/g, "O")
      .replace(/Ü/g, "U");

  const baseNorm = normalizeUmlaut(base);
  const pluralNorm = normalizeUmlaut(pl);
  const endings = ["en", "er", "e", "n", "s", ""];
  for (const ending of endings) {
    if (pluralNorm === `${baseNorm}${ending}`) {
      const hasUmlautShift = pl !== `${base}${ending}`;
      if (ending === "") {
        return hasUmlautShift ? "-¨" : "-";
      }
      return hasUmlautShift ? `-¨${ending}` : `-${ending}`;
    }
  }
  return "-";
}

function normalizeAdjectiveBase(result) {
  const title = emptyIfUnknown(result?.title);
  if (!title) {
    return "";
  }
  if (normalizeForKey(result?.sourceWord) !== normalizeForKey(title)) {
    return title;
  }

  const endings = ["en", "em", "er", "es", "e"];
  for (const ending of endings) {
    if (title.length > ending.length + 2 && title.toLowerCase().endsWith(ending)) {
      const candidate = title.slice(0, -ending.length);
      if (/^[A-Za-zÄÖÜäöüß-]{3,}$/.test(candidate)) {
        return candidate;
      }
    }
  }
  return title;
}

function isSentenceLikeExample(text) {
  const value = String(text || "").trim();
  if (!value) {
    return false;
  }
  if (value.length < 20) {
    return false;
  }
  if (/^[A-Za-zÄÖÜäöüß-]+:\s/.test(value)) {
    return false;
  }
  const words = value.split(/\s+/).filter(Boolean);
  if (words.length < 4) {
    return false;
  }
  const hasTerminalPunctuation = /[.!?]["'”»]*$/.test(value);
  return hasTerminalPunctuation || words.length >= 7;
}

function buildCardFromResult(result) {
  const translations = Array.isArray(result?.translations)
    ? result.translations.filter((item) => Boolean(emptyIfUnknown(item)))
    : [];
  if (translations.length === 0) {
    return null;
  }

  const partOfSpeech = emptyIfUnknown(result?.partOfSpeech);
  const title = emptyIfUnknown(result?.title);
  let front = title;

  if (partOfSpeech === "Verb") {
    const present = emptyIfUnknown(result?.verbInfo?.present) || title;
    const caseTag = getCaseShortLabel(result?.verbInfo?.caseGovernance);
    front = caseTag ? `${present} (${caseTag})` : present;
  } else if (partOfSpeech === "Noun") {
    const article = emptyIfUnknown(result?.nounInfo?.article);
    const plural = emptyIfUnknown(result?.nounInfo?.plural);
    const ending = derivePluralEnding(title, plural);
    const nounBase = article ? `${article} ${title}` : title;
    front = `${nounBase}, ${ending}`;
  } else if (partOfSpeech === "Adjective") {
    front = normalizeAdjectiveBase(result);
  }

  const selectedExamples = (Array.isArray(result?.examples) ? result.examples : [])
    .map((example) => String(example?.de || "").trim())
    .filter((text) => isSentenceLikeExample(text))
    .slice(0, 2)
    .map((text) => highlightExampleText(text, result));

  const backLines = [escapeHtml(translations.join(", "))];
  for (const exampleLine of selectedExamples) {
    backLines.push(`• ${exampleLine}`);
  }

  const dedupeKey = normalizeForKey(front);
  if (!dedupeKey) {
    return null;
  }

  return {
    ts: new Date().toISOString(),
    dedupeKey,
    front,
    back: backLines.join("<br>")
  };
}

function cardRichnessScore(card) {
  const back = String(card?.back || "");
  const exampleCount = (back.match(/<br>• /g) || []).length;
  return exampleCount * 1000 + back.length;
}

function sanitizeHistory(entries) {
  if (!Array.isArray(entries)) {
    return [];
  }
  return entries.filter((entry) => {
    const key = normalizeForKey(entry?.dedupeKey);
    const front = String(entry?.front || "").trim();
    const back = String(entry?.back || "").trim();
    return Boolean(key && front && back);
  });
}

async function appendLookupHistoryEntry(result) {
  if (!hasStorageApi()) {
    throw new Error("Storage-API ist nicht verfuegbar.");
  }
  const card = buildCardFromResult(result);
  if (!card) {
    return false;
  }

  const existing = sanitizeHistory(await readLookupHistory());
  const index = existing.findIndex(
    (entry) => normalizeForKey(entry?.dedupeKey) === card.dedupeKey
  );
  if (index === -1) {
    existing.push(card);
  } else if (cardRichnessScore(card) > cardRichnessScore(existing[index])) {
    existing[index] = card;
  }
  await writeLookupHistory(existing);
  return true;
}

function toTsvCell(value) {
  const text = String(value ?? "");
  if (!/["\t\n\r]/.test(text)) {
    return text;
  }
  return `"${text.replace(/"/g, "\"\"")}"`;
}

function serializeHistoryToTsv(entries) {
  const rows = sanitizeHistory(entries).map((entry) => [
    entry.front,
    entry.back
  ]);

  const body = rows
    .map((row) => row.map((cell) => toTsvCell(cell)).join("\t"))
    .join("\n");
  return `\uFEFF${body}`;
}

function buildExportFilename() {
  const now = new Date();
  const yyyy = String(now.getFullYear());
  const mm = String(now.getMonth() + 1).padStart(2, "0");
  const dd = String(now.getDate()).padStart(2, "0");
  return `anki_export_${yyyy}-${mm}-${dd}.tsv`;
}

function downloadTsv(content, filename) {
  return new Promise((resolve, reject) => {
    if (!hasDownloadsApi()) {
      reject(new Error("Downloads-API ist nicht verfuegbar."));
      return;
    }

    const blob = new Blob([content], {
      type: "text/tab-separated-values;charset=utf-8"
    });
    const url = URL.createObjectURL(blob);
    chrome.downloads.download(
      {
        url,
        filename,
        saveAs: true,
        conflictAction: "uniquify"
      },
      (downloadId) => {
        setTimeout(() => URL.revokeObjectURL(url), 1000);
        if (chrome.runtime?.lastError) {
          reject(new Error(chrome.runtime.lastError.message));
          return;
        }
        if (typeof downloadId !== "number") {
          reject(new Error("Download wurde nicht gestartet."));
          return;
        }
        resolve(downloadId);
      }
    );
  });
}

async function exportHistoryAsTsv() {
  if (exportInFlight) {
    return;
  }
  exportInFlight = true;
  if (exportBtn) {
    exportBtn.disabled = true;
  }

  try {
    if (!hasStorageApi()) {
      throw new Error("Speicherberechtigung wird fuer den Export benoetigt.");
    }
    if (!hasDownloadsApi()) {
      throw new Error("Download-Berechtigung wird fuer den Export benoetigt.");
    }

    const history = sanitizeHistory(await readLookupHistory());
    if (history.length === 0) {
      statusEl.textContent = "Keine exportierbaren Karten im Verlauf.";
      return;
    }

    const tsv = serializeHistoryToTsv(history);
    const filename = buildExportFilename();
    await downloadTsv(tsv, filename);
    await writeLookupHistory([]);
    statusEl.textContent = `${history.length} Karten als TSV exportiert.`;
  } catch (error) {
    statusEl.textContent = `Export fehlgeschlagen: ${error.message || "Unbekannter Fehler."}`;
  } finally {
    exportInFlight = false;
    if (exportBtn) {
      exportBtn.disabled = false;
    }
  }
}

function unknownBadge(value) {
  if (value !== "Unknown") {
    return escapeHtml(value);
  }
  return `Unbekannt <span class="badge-unknown">fehlt</span>`;
}

function buildConjugationTooltip(conjugation) {
  if (!conjugation) {
    return "";
  }

  const rows = Object.entries(conjugation)
    .map(
      ([person, value]) =>
        `<div>${escapeHtml(person)}: ${escapeHtml(value || "Unbekannt")}</div>`
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

function hideTooltip(target) {
  const tooltip = target?.querySelector(".tooltip");
  if (!tooltip) {
    return;
  }
  tooltip.classList.remove("tooltip-visible");
}

function positionTooltip(target) {
  const tooltip = target?.querySelector(".tooltip");
  if (!tooltip) {
    return;
  }

  tooltip.classList.add("tooltip-visible");
  tooltip.style.left = "0px";
  tooltip.style.top = "0px";

  const margin = 8;
  const gap = 8;
  const targetRect = target.getBoundingClientRect();
  const tooltipRect = tooltip.getBoundingClientRect();

  let left = targetRect.left;
  let top = targetRect.top - tooltipRect.height - gap;

  if (left + tooltipRect.width > window.innerWidth - margin) {
    left = window.innerWidth - margin - tooltipRect.width;
  }
  if (left < margin) {
    left = margin;
  }

  if (top < margin) {
    top = targetRect.bottom + gap;
  }
  if (top + tooltipRect.height > window.innerHeight - margin) {
    top = window.innerHeight - margin - tooltipRect.height;
  }
  if (top < margin) {
    top = margin;
  }

  tooltip.style.left = `${Math.round(left)}px`;
  tooltip.style.top = `${Math.round(top)}px`;
}

function attachTooltipHandlers() {
  const hoverTargets = resultEl.querySelectorAll(".hover-target");
  for (const target of hoverTargets) {
    target.addEventListener("mouseenter", () => positionTooltip(target));
    target.addEventListener("mouseleave", () => hideTooltip(target));
    target.addEventListener("focusin", () => positionTooltip(target));
    target.addEventListener("focusout", () => hideTooltip(target));
  }
}

function renderIdle(message) {
  statusEl.textContent = message;
  resultEl.innerHTML = "";
}

function renderLoading(word) {
  statusEl.textContent = `Suche nach \"${word}\"...`;
  resultEl.innerHTML = "";
}

function renderError(message) {
  statusEl.textContent = "Fehler";
  resultEl.innerHTML = `<div class="card"><div class="row">${escapeHtml(message)}</div></div>`;
}

function renderNotFound(word) {
  statusEl.textContent = "Kein Eintrag gefunden";
  resultEl.innerHTML = `<div class="card"><div class="row">Kein Wiktionary-Eintrag fuer \"${escapeHtml(word)}\" gefunden.</div></div>`;
}

function renderSuccess(result) {
  statusEl.textContent = "";

  const translations = Array.isArray(result?.translations)
    ? result.translations
    : [];

  const translationsHtml =
    translations.length > 0
      ? `<div class="row">${translations
          .map((value) => escapeHtml(value))
          .join(", ")}</div>`
      : `<div class="row">Keine englischen Uebersetzungen gefunden.</div>`;

  const caseShort = getCaseShortLabel(result?.verbInfo?.caseGovernance);
  const caseTag =
    result.partOfSpeech === "Verb" && caseShort
      ? ` <span class="case-tag">(${escapeHtml(caseShort)})</span>`
      : "";

  const nounArticle = emptyIfUnknown(result?.nounInfo?.article);
  const nounPlural = emptyIfUnknown(result?.nounInfo?.plural);
  let headwordLine = `${escapeHtml(result.title)} <span class="pos-inline">(${escapeHtml(
    localizePartOfSpeech(result.partOfSpeech)
  )})</span>${caseTag}`;
  if (result.partOfSpeech === "Noun") {
    const singularDisplay = nounArticle
      ? `${nounArticle} ${result.title}`
      : result.title;
    const pluralDisplay = nounPlural
      ? `(pl. ${nounArticle || ""}${nounArticle ? " " : ""}${nounPlural})`
      : "";
    headwordLine = `${escapeHtml(singularDisplay)}${
      pluralDisplay ? ` <span class="pos-inline">${escapeHtml(pluralDisplay)}</span>` : ""
    }`;
  }

  const verbHtml = result.verbInfo
    ? `
      <div class="card">
        <h2>Verb</h2>
        ${
          result.verbInfo.caseGovernance &&
          result.verbInfo.caseGovernance !== "Unknown"
            ? `<div class="row"><span class="label">Kasus:</span> ${escapeHtml(result.verbInfo.caseGovernance)}</div>`
            : ""
        }
        ${
          Array.isArray(result.verbInfo.infinitiveForms) &&
          result.verbInfo.infinitiveForms.length > 0
            ? `<div class="row"><span class="label">Formen:</span> ${result.verbInfo.infinitiveForms
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
            <tr>
              <th>Imperativ</th>
              <td colspan="2">${unknownBadge(result.verbInfo.imperative || "Unknown")}</td>
            </tr>
          </tbody>
        </table>
      </div>
    `
    : "";

  const notesHtml =
    result.notes && result.notes.length
      ? `<div class="card"><h2>Hinweise</h2><ul class="notes">${result.notes
          .map((note) => `<li>${escapeHtml(note)}</li>`)
          .join("")}</ul></div>`
      : "";

  const examples = Array.isArray(result?.examples) ? result.examples : [];
  const examplesHtml =
    examples.length > 0
      ? `
      <div class="card">
        <h2>Beispiele</h2>
        <ul class="notes">
          ${examples
            .slice(0, 5)
            .map((example) => {
              const de = highlightExampleText(example?.de || "", result);
              const en = example?.en ? ` <span class="example-en">- ${escapeHtml(example.en)}</span>` : "";
              return de ? `<li>${de}${en}</li>` : "";
            })
            .join("")}
        </ul>
      </div>
    `
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
        <h2>Abgeleitete Begriffe</h2>
        <div class="derived-list">
          ${derivedVariantStates
            .map((item) => {
              const term = escapeHtml(item.term);
              if (item.clickable) {
                return `<button class="derived-btn" type="button" data-variant="${term}">${term}</button>`;
              }
              return `<button class="derived-btn disabled" type="button" disabled title="Keine englische Uebersetzung gefunden">${term}</button>`;
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
    ${verbHtml}
    ${examplesHtml}
    ${derivedHtml}
    ${notesHtml}
  `;

  attachTooltipHandlers();
}

function speakWord(word) {
  const normalized = normalizeLookupWord(word);
  if (!normalized) {
    return;
  }

  if (!("speechSynthesis" in window)) {
    statusEl.textContent = "Sprachausgabe ist in diesem Browser nicht verfuegbar.";
    return;
  }

  window.speechSynthesis.cancel();
  const utterance = new SpeechSynthesisUtterance(normalized);
  utterance.lang = "de-DE";
  utterance.rate = 0.95;
  utterance.onerror = () => {
    statusEl.textContent = "Aussprache konnte nicht abgespielt werden.";
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
    renderIdle("Gib ein deutsches Wort zum Uebersetzen ein.");
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
      appendLookupHistoryEntry(response.result).catch((error) => {
        console.warn("Failed to append lookup history:", error);
      });
      lastCompletedWord = normalized;
    }
  } catch (error) {
    if (token !== requestToken) {
      return;
    }
    renderError(error.message || "Suche fehlgeschlagen.");
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

if (exportBtn) {
  exportBtn.addEventListener("click", () => {
    exportHistoryAsTsv();
  });
}

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
  renderIdle("Waehle ein deutsches Wort auf einer Seite aus oder gib es oben ein.");
  input.focus();
  const selectedWord = await getHighlightedWord();
  if (selectedWord) {
    input.value = selectedWord;
    input.select();
    runLookup(selectedWord);
  }
});
