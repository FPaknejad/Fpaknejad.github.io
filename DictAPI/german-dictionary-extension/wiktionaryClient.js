import {
  extractLemmaCandidate,
  hasInflectedFormMarkers,
  parseEntry
} from "./parser.js";

const BASE_URL = "https://de.wiktionary.org/w/api.php";
const API_USER_AGENT =
  "GermanDictionaryExtension/1.0 (local Chrome extension; contact: local-user)";

function normalizeTitle(value) {
  return String(value || "")
    .trim()
    .toLocaleLowerCase("de-DE");
}

function isMeaningfulValue(value) {
  return value !== undefined && value !== null && value !== "" && value !== "Unknown";
}

function mergeUnknownFields(primary, fallback) {
  if (primary == null) {
    return fallback ?? primary;
  }
  if (fallback == null) {
    return primary;
  }

  if (Array.isArray(primary)) {
    return primary.length > 0 ? primary : fallback;
  }

  if (typeof primary !== "object" || typeof fallback !== "object") {
    return !isMeaningfulValue(primary) && isMeaningfulValue(fallback)
      ? fallback
      : primary;
  }

  const merged = { ...primary };
  for (const [key, fallbackValue] of Object.entries(fallback)) {
    const currentValue = merged[key];
    merged[key] = mergeUnknownFields(currentValue, fallbackValue);
  }
  return merged;
}

async function fetchJson(params) {
  const url = `${BASE_URL}?${new URLSearchParams(params).toString()}`;
  let response;
  try {
    response = await fetch(url, {
      method: "GET",
      headers: {
        Accept: "application/json",
        "User-Agent": API_USER_AGENT,
        "Api-User-Agent": API_USER_AGENT
      }
    });
  } catch (error) {
    throw new Error(`Wiktionary network request failed: ${error.message}`);
  }

  if (!response.ok) {
    throw new Error(`Wiktionary API request failed: HTTP ${response.status}`);
  }

  try {
    return await response.json();
  } catch {
    throw new Error("Wiktionary API returned invalid JSON.");
  }
}

async function searchTitles(word) {
  const data = await fetchJson({
    action: "query",
    list: "search",
    srsearch: word,
    srlimit: "5",
    format: "json",
    formatversion: "2",
    utf8: "1"
  });
  return data?.query?.search || [];
}

function pickBestTitle(results, word) {
  if (!results.length) {
    return null;
  }
  const normalized = normalizeTitle(word);
  const exact = results.find((item) => normalizeTitle(item.title) === normalized);
  return exact ? exact.title : results[0].title;
}

async function fetchWikitext(title) {
  const data = await fetchJson({
    action: "parse",
    page: title,
    prop: "wikitext",
    format: "json",
    formatversion: "2",
    utf8: "1"
  });
  return data?.parse?.wikitext || "";
}

async function checkTitleExists(title) {
  try {
    const data = await fetchJson({
      action: "query",
      titles: title,
      format: "json",
      formatversion: "2",
      utf8: "1"
    });
    const pages = data?.query?.pages;
    if (Array.isArray(pages) && pages.length > 0 && !pages[0].missing) {
      return true;
    }
  } catch {
    // Fall through to parse-based existence check.
  }

  try {
    const wikitext = await fetchWikitext(title);
    return Boolean(String(wikitext || "").trim());
  } catch {
    return false;
  }
}

async function validateDerivedVariants(candidates, limit = 20) {
  if (!Array.isArray(candidates) || candidates.length === 0) {
    return [];
  }

  const valid = [];
  const seen = new Set();
  for (const candidate of candidates.slice(0, limit)) {
    const term = String(candidate || "").trim();
    if (!term) {
      continue;
    }
    const key = normalizeTitle(term);
    if (seen.has(key)) {
      continue;
    }
    seen.add(key);
    try {
      const exists = await checkTitleExists(term);
      if (exists) {
        valid.push(term);
      }
    } catch {
      // Ignore individual existence check failures to avoid blocking lookup.
    }
  }
  return valid;
}

async function checkVariantHasEnglishTranslations(term) {
  const results = await searchTitles(term);
  const bestTitle = pickBestTitle(results, term);
  if (!bestTitle) {
    return false;
  }

  const wikitext = await fetchWikitext(bestTitle);
  const parsed = parseEntry({
    sourceWord: term,
    title: bestTitle,
    wikitext
  });
  return Array.isArray(parsed.translations) && parsed.translations.length > 0;
}

async function buildDerivedVariantStates(candidates, limit = 30) {
  if (!Array.isArray(candidates) || candidates.length === 0) {
    return [];
  }

  const states = [];
  const seen = new Set();

  for (const candidate of candidates.slice(0, limit)) {
    const term = String(candidate || "").trim();
    if (!term) {
      continue;
    }
    const key = normalizeTitle(term);
    if (seen.has(key)) {
      continue;
    }
    seen.add(key);

    let clickable = false;
    try {
      clickable = await checkVariantHasEnglishTranslations(term);
    } catch {
      clickable = false;
    }

    states.push({ term, clickable });
  }

  return states;
}

function mergeResult(primary, fallback, note) {
  const mergedVerbInfo = mergeUnknownFields(primary.verbInfo, fallback.verbInfo);
  const mergedNounInfo = mergeUnknownFields(primary.nounInfo, fallback.nounInfo);
  const merged = {
    ...primary,
    translations:
      primary.translations.length > 0
        ? primary.translations
        : fallback.translations,
    nounInfo: mergedNounInfo,
    verbInfo: mergedVerbInfo,
    notes: [...(primary.notes || [])]
  };
  if (note) {
    merged.notes.push(note);
  }
  return merged;
}

export async function lookupGermanWord(word) {
  const sourceWord = (word || "").trim();
  if (!sourceWord) {
    throw new Error("Lookup word is empty.");
  }

  const results = await searchTitles(sourceWord);
  const title = pickBestTitle(results, sourceWord);
  if (!title) {
    return { status: "not_found", word: sourceWord };
  }

  const wikitext = await fetchWikitext(title);
  let parsed = parseEntry({
    sourceWord,
    title,
    wikitext
  });

  const sourceLower = sourceWord.toLowerCase();
  const titleLower = title.toLowerCase();
  const hasFormReference = hasInflectedFormMarkers(wikitext);

  // Only try lemma normalization when lookup target differs from source,
  // for verb entries, or for unknown-pos entries that explicitly look inflected.
  const shouldResolveLemma =
    sourceLower !== titleLower ||
    parsed.partOfSpeech === "Verb" ||
    (parsed.partOfSpeech === "Unknown" && hasFormReference);

  const lemma = shouldResolveLemma ? extractLemmaCandidate(wikitext, title) : null;
  if (lemma && lemma.toLowerCase() !== titleLower) {
    try {
      const lemmaWikitext = await fetchWikitext(lemma);
      const lemmaParsed = parseEntry({
        sourceWord,
        title: lemma,
        wikitext: lemmaWikitext,
        fallback: parsed
      });
      parsed = mergeResult(
        lemmaParsed,
        parsed,
        `Resolved inflected form to lemma: ${lemma}`
      );
    } catch (error) {
      parsed.notes.push(`Lemma lookup failed: ${error.message}`);
    }
  }

  try {
    const variantStates = await buildDerivedVariantStates(
      parsed.derivedVariants,
      30
    );
    parsed.derivedVariantStates = variantStates;
    parsed.validDerivedVariants = variantStates
      .filter((item) => item.clickable)
      .map((item) => item.term);
  } catch {
    parsed.derivedVariantStates = [];
    parsed.validDerivedVariants = [];
  }

  return { status: "ok", result: parsed };
}
