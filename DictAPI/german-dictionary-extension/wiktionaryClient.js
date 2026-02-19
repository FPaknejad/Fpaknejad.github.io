import {
  extractLemmaCandidate,
  hasInflectedFormMarkers,
  parseEntry
} from "./parser.js";

const BASE_URL = "https://de.wiktionary.org/w/api.php";
const API_USER_AGENT =
  "GermanDictionaryExtension/1.0 (local Chrome extension; contact: local-user)";

async function fetchJson(params) {
  const url = `${BASE_URL}?${new URLSearchParams(params).toString()}`;
  const response = await fetch(url, {
    method: "GET",
    headers: {
      Accept: "application/json",
      "User-Agent": API_USER_AGENT,
      "Api-User-Agent": API_USER_AGENT
    }
  });

  if (!response.ok) {
    throw new Error(`API request failed: HTTP ${response.status}`);
  }
  return response.json();
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
  const normalized = word.trim().toLowerCase();
  const exact = results.find((item) => item.title.toLowerCase() === normalized);
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

function mergeResult(primary, fallback, note) {
  const merged = {
    ...primary,
    translations:
      primary.translations.length > 0
        ? primary.translations
        : fallback.translations,
    nounInfo: primary.nounInfo || fallback.nounInfo,
    verbInfo: primary.verbInfo || fallback.verbInfo,
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

  return { status: "ok", result: parsed };
}
