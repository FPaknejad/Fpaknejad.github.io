import {
  extractLemmaCandidate,
  hasInflectedFormMarkers,
  parseEntry
} from "./parser.js";

const BASE_URL = "https://de.wiktionary.org/w/api.php";
const API_USER_AGENT =
  "GermanDictionaryExtension/1.0 (local Chrome extension; contact: local-user)";
const variantTranslationCache = new Map();
const derivedVariantStateCache = new Map();

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
  const cacheKey = normalizeTitle(term);
  if (variantTranslationCache.has(cacheKey)) {
    return variantTranslationCache.get(cacheKey);
  }

  let hasTranslations = false;
  try {
    // Use direct parse lookup to avoid extra search/query round-trips.
    const wikitext = await fetchWikitext(term);
    const parsed = parseEntry({
      sourceWord: term,
      title: term,
      wikitext
    });
    hasTranslations =
      Array.isArray(parsed.translations) && parsed.translations.length > 0;
  } catch {
    hasTranslations = false;
  }

  variantTranslationCache.set(cacheKey, hasTranslations);
  return hasTranslations;
}

async function mapWithConcurrency(items, limit, mapper) {
  const result = new Array(items.length);
  let nextIndex = 0;

  async function worker() {
    while (true) {
      const current = nextIndex;
      nextIndex += 1;
      if (current >= items.length) {
        return;
      }
      result[current] = await mapper(items[current], current);
    }
  }

  const workers = [];
  const size = Math.max(1, Math.min(limit, items.length));
  for (let i = 0; i < size; i += 1) {
    workers.push(worker());
  }
  await Promise.all(workers);
  return result;
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

    states.push({ term, clickable: false });
  }

  const clickableFlags = await mapWithConcurrency(
    states.map((item) => item.term),
    6,
    async (term) => checkVariantHasEnglishTranslations(term)
  );

  return states.map((item, index) => ({
    term: item.term,
    clickable: Boolean(clickableFlags[index])
  }));
}

function mergeResult(primary, fallback, note) {
  const mergedExamples = mergeUnknownFields(primary.examples, fallback.examples);
  const mergedVerbInfo = mergeUnknownFields(primary.verbInfo, fallback.verbInfo);
  const mergedNounInfo = mergeUnknownFields(primary.nounInfo, fallback.nounInfo);
  const merged = {
    ...primary,
    translations:
      primary.translations.length > 0
        ? primary.translations
        : fallback.translations,
    examples: mergedExamples,
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
    const derivedCacheKey = normalizeTitle(parsed.title);
    let variantStates = derivedVariantStateCache.get(derivedCacheKey);
    if (!variantStates) {
      variantStates = await buildDerivedVariantStates(parsed.derivedVariants, 30);
      derivedVariantStateCache.set(derivedCacheKey, variantStates);
    }
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
