function escapeHtml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/\"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

function escapeRegExp(value) {
  return String(value).replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

const SEPARABLE_PREFIXES = new Set([
  "ab",
  "an",
  "auf",
  "aus",
  "bei",
  "ein",
  "empor",
  "entgegen",
  "fern",
  "fest",
  "fort",
  "frei",
  "gegenueber",
  "gegenüber",
  "gleich",
  "heim",
  "her",
  "herab",
  "heran",
  "herauf",
  "heraus",
  "herein",
  "herum",
  "herunter",
  "hin",
  "hinab",
  "hinauf",
  "hinaus",
  "hinein",
  "hinterher",
  "hinweg",
  "hoch",
  "los",
  "mit",
  "nach",
  "nieder",
  "preis",
  "vor",
  "voran",
  "vorbei",
  "vorweg",
  "weg",
  "weiter",
  "wieder",
  "zurecht",
  "zurueck",
  "zurück",
  "zusammen",
  "zu"
]);

function isUnknown(value) {
  return !value || String(value).trim().toLowerCase() === "unknown";
}

function applyOnTextNodes(html, replacer) {
  return html
    .split(/(<[^>]+>)/g)
    .map((part) => (part.startsWith("<") ? part : replacer(part)))
    .join("");
}

function addToken(tokenSet, value) {
  const token = String(value || "").trim();
  if (isUnknown(token) || token.includes(" ")) {
    return;
  }
  tokenSet.add(token);
}

function parseSeparablePair(value) {
  const text = String(value || "").trim();
  if (!text) {
    return null;
  }
  const parts = text.split(/\s+/).filter(Boolean);
  if (parts.length !== 2) {
    return null;
  }
  const [left, right] = parts;
  if (left.length < 2 || right.length < 2) {
    return null;
  }
  if (!SEPARABLE_PREFIXES.has(right.toLowerCase())) {
    return null;
  }
  return { left, right };
}

function buildMatchInventory(result) {
  const tokens = new Set();
  const separablePairs = [];
  const separablePrefixes = new Set();

  addToken(tokens, result?.sourceWord);
  addToken(tokens, result?.title);

  const verbInfo = result?.verbInfo;
  if (verbInfo) {
    addToken(tokens, verbInfo.present);
    addToken(tokens, verbInfo.preterite);
    addToken(tokens, verbInfo.pastParticiple);

    if (Array.isArray(verbInfo.infinitiveForms)) {
      for (const form of verbInfo.infinitiveForms) {
        addToken(tokens, form);
        const pair = parseSeparablePair(form);
        if (pair) {
          separablePairs.push(pair);
          separablePrefixes.add(pair.right.toLowerCase());
        }
      }
    }

    const conjugations = [
      verbInfo.presentConjugation,
      verbInfo.preteriteConjugation
    ];
    for (const conjugation of conjugations) {
      if (!conjugation || typeof conjugation !== "object") {
        continue;
      }
      for (const value of Object.values(conjugation)) {
        addToken(tokens, value);
        const pair = parseSeparablePair(value);
        if (pair) {
          separablePairs.push(pair);
          separablePrefixes.add(pair.right.toLowerCase());
          const joined = `${pair.right}${pair.left}`;
          tokens.add(joined);
          // Common plural/simple inflection coverage for preterite-like joins
          // (e.g. "ankam" -> "ankamen").
          if (!joined.toLowerCase().endsWith("en")) {
            tokens.add(`${joined}en`);
          }
        }
      }
    }
  }

  const title = String(result?.title || "").trim();
  if (title && result?.partOfSpeech === "Verb") {
    for (const prefix of separablePrefixes) {
      if (title.toLowerCase().startsWith(prefix) && title.length > prefix.length + 1) {
        const base = title.slice(prefix.length);
        tokens.add(`${prefix}zu${base}`);
      }
    }
  }

  return {
    tokens: [...tokens].sort((a, b) => b.length - a.length),
    separablePairs
  };
}

function highlightAdjectiveInflections(html, result) {
  if (result?.partOfSpeech !== "Adjective") {
    return html;
  }
  const lemma = String(result?.title || "").trim();
  if (!/^[A-Za-zÄÖÜäöüß]+$/.test(lemma)) {
    return html;
  }

  const pattern = new RegExp(
    `\\b(${escapeRegExp(lemma)}(?:e|en|em|er|es))\\b`,
    "gi"
  );
  return applyOnTextNodes(html, (text) => text.replace(pattern, "<strong>$1</strong>"));
}

export function highlightExampleText(text, result) {
  const raw = String(text || "");
  if (!raw) {
    return "";
  }

  const { tokens, separablePairs } = buildMatchInventory(result);
  let html = escapeHtml(raw);

  for (const { left, right } of separablePairs) {
    const pairRegex = new RegExp(
      `\\b(${escapeRegExp(left)})\\b([\\s\\S]{0,120}?)\\b(${escapeRegExp(right)})\\b`,
      "gi"
    );
    html = applyOnTextNodes(html, (segment) =>
      segment.replace(
        pairRegex,
        (_match, first, middle, second) =>
          `<strong>${first}</strong>${middle}<strong>${second}</strong>`
      )
    );
  }

  for (const token of tokens) {
    const tokenRegex = new RegExp(`\\b(${escapeRegExp(token)})\\b`, "gi");
    html = applyOnTextNodes(html, (segment) =>
      segment.replace(tokenRegex, "<strong>$1</strong>")
    );
  }

  html = highlightAdjectiveInflections(html, result);
  return html;
}
