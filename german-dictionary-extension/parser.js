const UNKNOWN = "Unknown";

function cleanupValue(value) {
  if (!value) {
    return UNKNOWN;
  }

  let cleaned = value.trim();
  cleaned = cleaned.replace(/<!--.*?-->/g, "");
  cleaned = cleaned.replace(/<ref[^>]*>[\s\S]*?<\/ref>/gi, "");
  cleaned = cleaned.replace(/<[^>]+>/g, "");
  cleaned = cleaned.replace(/\[\[([^|\]]+)\|([^\]]+)\]\]/g, "$2");
  cleaned = cleaned.replace(/\[\[([^\]]+)\]\]/g, "$1");
  cleaned = cleaned.replace(/{{[^{}]*}}/g, "");
  cleaned = cleaned.replace(/'''+|''/g, "");
  cleaned = cleaned.replace(/\s+/g, " ").trim();

  if (!cleaned || cleaned === "-" || cleaned === "—") {
    return UNKNOWN;
  }
  return cleaned;
}

function extractSection(wikitext, heading) {
  const pattern = new RegExp(
    `==+\\s*${heading}\\s*==+([\\s\\S]*?)(\\n==+[^=\\n]+==+|$)`,
    "i"
  );
  const match = wikitext.match(pattern);
  return match ? match[1] : "";
}

function extractGrammarSection(wikitext) {
  return extractSection(wikitext, "Grammatische Merkmale");
}

function extractMeaningsSection(wikitext) {
  return extractSection(wikitext, "Bedeutungen");
}

function dedupe(items) {
  return [...new Set(items)];
}

function isUnknown(value) {
  return !value || value === UNKNOWN;
}

function normalizeFieldName(name) {
  return String(name)
    .toLowerCase()
    .replace(/[,\s_/.-]+/g, "");
}

function buildFieldMap(wikitext) {
  const map = new Map();
  const regex = /^\|\s*([^=\n]+?)\s*=\s*([^\n]*)/gm;
  let match;

  while ((match = regex.exec(wikitext)) !== null) {
    const key = normalizeFieldName(match[1]);
    const value = cleanupValue(match[2]);
    if (isUnknown(value)) {
      continue;
    }

    const existing = map.get(key) || [];
    if (!existing.includes(value)) {
      existing.push(value);
    }
    map.set(key, existing);
  }

  return map;
}

function pickFirstFieldFromMap(fieldMap, keys) {
  for (const key of keys) {
    const values = fieldMap.get(normalizeFieldName(key));
    if (values && values.length) {
      return values[0];
    }
  }
  return UNKNOWN;
}

function pickFirstField(wikitext, keys) {
  const fieldMap = buildFieldMap(wikitext);
  return pickFirstFieldFromMap(fieldMap, keys);
}

function extractConjugation(fieldMap, fieldAliases) {
  const result = {};
  let hasAny = false;

  for (const [person, keys] of Object.entries(fieldAliases)) {
    const value = pickFirstFieldFromMap(fieldMap, keys);
    result[person] = value;
    if (!isUnknown(value)) {
      hasAny = true;
    }
  }

  return hasAny ? result : undefined;
}

function cloneConjugation(value) {
  return value ? { ...value } : undefined;
}

function fillPresentFallback(conjugation, infinitive) {
  const result = cloneConjugation(conjugation) || {
    ich: UNKNOWN,
    du: UNKNOWN,
    "er/sie/es": UNKNOWN,
    wir: UNKNOWN,
    ihr: UNKNOWN,
    sie: UNKNOWN
  };

  const inf = (infinitive || "").toLowerCase();
  if (!inf) {
    return result;
  }

  if (isUnknown(result.wir)) {
    if (inf === "sein") {
      result.wir = "sind";
    } else {
      result.wir = infinitive;
    }
  }

  if (isUnknown(result.sie) && !isUnknown(result.wir)) {
    result.sie = result.wir;
  }

  if (isUnknown(result.ihr)) {
    if (inf === "sein") {
      result.ihr = "seid";
    } else if (inf === "haben") {
      result.ihr = "habt";
    } else if (!isUnknown(result.wir) && /en$/i.test(result.wir)) {
      result.ihr = result.wir.replace(/en$/i, "t");
    } else if (!isUnknown(result.du) && /st$/i.test(result.du)) {
      result.ihr = result.du.replace(/st$/i, "t");
    }
  }

  return result;
}

function fillPreteriteFallback(conjugation, infinitive) {
  const result = cloneConjugation(conjugation) || {
    ich: UNKNOWN,
    du: UNKNOWN,
    "er/sie/es": UNKNOWN,
    wir: UNKNOWN,
    ihr: UNKNOWN,
    sie: UNKNOWN
  };

  const inf = (infinitive || "").toLowerCase();
  if (inf === "sein") {
    if (isUnknown(result.ich)) result.ich = "war";
    if (isUnknown(result.du)) result.du = "warst";
    if (isUnknown(result["er/sie/es"])) result["er/sie/es"] = "war";
    if (isUnknown(result.wir)) result.wir = "waren";
    if (isUnknown(result.ihr)) result.ihr = "wart";
    if (isUnknown(result.sie)) result.sie = "waren";
    return result;
  }
  if (inf === "haben") {
    if (isUnknown(result.ich)) result.ich = "hatte";
    if (isUnknown(result.du)) result.du = "hattest";
    if (isUnknown(result["er/sie/es"])) result["er/sie/es"] = "hatte";
    if (isUnknown(result.wir)) result.wir = "hatten";
    if (isUnknown(result.ihr)) result.ihr = "hattet";
    if (isUnknown(result.sie)) result.sie = "hatten";
    return result;
  }

  const ich = !isUnknown(result.ich) ? result.ich : null;
  if (!isUnknown(result["er/sie/es"]) && !ich) {
    // prefer reusing er/sie/es as the singular base
    result.ich = result["er/sie/es"];
  }

  const singularBase = !isUnknown(result.ich)
    ? result.ich
    : !isUnknown(result["er/sie/es"])
      ? result["er/sie/es"]
      : null;

  if (!singularBase) {
    return result;
  }

  if (isUnknown(result["er/sie/es"])) {
    result["er/sie/es"] = singularBase;
  }

  if (isUnknown(result.wir)) {
    if (/te$/i.test(singularBase)) {
      result.wir = `${singularBase}n`;
    } else {
      result.wir = `${singularBase}en`;
    }
  }

  if (isUnknown(result.sie) && !isUnknown(result.wir)) {
    result.sie = result.wir;
  }

  if (isUnknown(result.du)) {
    if (/te$/i.test(singularBase)) {
      result.du = `${singularBase}st`;
    } else {
      result.du = `${singularBase}st`;
    }
  }

  if (isUnknown(result.ihr)) {
    if (!isUnknown(result.wir) && /en$/i.test(result.wir)) {
      result.ihr = result.wir.replace(/en$/i, "t");
    } else if (/te$/i.test(singularBase)) {
      result.ihr = `${singularBase}t`;
    } else {
      result.ihr = `${singularBase}t`;
    }
  }

  return result;
}

function extractAuxiliaryVerbs(fieldMap) {
  const keys = [
    "Hilfsverb",
    "Hilfsverb_1",
    "Hilfsverb_2",
    "Perfekthilfsverb"
  ];

  const values = [];
  for (const key of keys) {
    const found = fieldMap.get(normalizeFieldName(key)) || [];
    for (const value of found) {
      if (!values.includes(value)) {
        values.push(value);
      }
    }
  }
  return values;
}

function buildPerfectConjugation(auxiliaryVerbs, participle) {
  if (isUnknown(participle) || !auxiliaryVerbs.length) {
    return undefined;
  }

  const auxToForms = {
    haben: {
      ich: "habe",
      du: "hast",
      "er/sie/es": "hat",
      wir: "haben",
      ihr: "habt",
      sie: "haben"
    },
    sein: {
      ich: "bin",
      du: "bist",
      "er/sie/es": "ist",
      wir: "sind",
      ihr: "seid",
      sie: "sind"
    }
  };

  const persons = ["ich", "du", "er/sie/es", "wir", "ihr", "sie"];
  const result = {};
  let hasAny = false;

  for (const person of persons) {
    const auxForms = [];
    for (const auxRaw of auxiliaryVerbs) {
      const aux = auxRaw.toLowerCase();
      const form = auxToForms[aux]?.[person];
      if (form && !auxForms.includes(form)) {
        auxForms.push(form);
      }
    }

    if (auxForms.length) {
      result[person] = `${auxForms.join("/")} ${participle}`;
      hasAny = true;
    } else {
      result[person] = UNKNOWN;
    }
  }

  return hasAny ? result : undefined;
}

export function detectPartOfSpeech(wikitext) {
  const checks = [
    { pattern: /{{\s*Wortart\|Verb\|Deutsch/i, value: "Verb" },
    { pattern: /{{\s*Wortart\|Substantiv\|Deutsch/i, value: "Noun" },
    { pattern: /{{\s*Wortart\|Adjektiv\|Deutsch/i, value: "Adjective" }
  ];

  for (const check of checks) {
    if (check.pattern.test(wikitext)) {
      return check.value;
    }
  }
  return "Unknown";
}

export function extractTranslations(wikitext) {
  const section = extractSection(wikitext, "Übersetzungen");
  if (!section) {
    return [];
  }

  const matches = [];
  const regex = /{{Ü[^|}]*\|en\|([^}|]+)[^}]*}}/g;
  let match;
  while ((match = regex.exec(section)) !== null) {
    const cleaned = cleanupValue(match[1]);
    if (!isUnknown(cleaned)) {
      matches.push(cleaned);
    }
  }

  return dedupe(matches).slice(0, 10);
}

export function extractNounInfo(wikitext) {
  const hasNounMarker =
    /{{\s*Wortart\|Substantiv\|Deutsch/i.test(wikitext) ||
    /Deutsch Substantiv Übersicht/i.test(wikitext);
  if (!hasNounMarker) {
    return undefined;
  }

  let genus = UNKNOWN;
  if (/\|\s*Genus\s*=\s*m\b/i.test(wikitext) || /{{\s*m\s*}}/.test(wikitext)) {
    genus = "der";
  } else if (
    /\|\s*Genus\s*=\s*f\b/i.test(wikitext) ||
    /{{\s*f\s*}}/.test(wikitext)
  ) {
    genus = "die";
  } else if (
    /\|\s*Genus\s*=\s*n\b/i.test(wikitext) ||
    /{{\s*n\s*}}/.test(wikitext)
  ) {
    genus = "das";
  }

  const plural = pickFirstField(wikitext, [
    "Nominativ Plural",
    "Plural"
  ]);

  return {
    article: genus,
    plural
  };
}

function normalizeGovernanceDetail(detail) {
  return detail
    .replace(/\s+/g, " ")
    .replace(/\bakkusativ\b/gi, "Akkusativ")
    .replace(/akk\./gi, "Akkusativ")
    .replace(/\bdativ\b/gi, "Dativ")
    .replace(/dat\./gi, "Dativ")
    .replace(/\bgenitiv\b/gi, "Genitiv")
    .replace(/gen\./gi, "Genitiv")
    .trim();
}

function toSearchableText(value) {
  return String(value)
    .replace(/\[\[([^|\]]+)\|([^\]]+)\]\]/g, "$2")
    .replace(/\[\[([^\]]+)\]\]/g, "$1")
    .replace(/<[^>]+>/g, " ")
    .replace(/'''+|''/g, "")
    .replace(/\s+/g, " ");
}

function extractCaseGovernanceDetails(wikitext) {
  const grammarSection = extractGrammarSection(wikitext);
  const meaningsSection = extractMeaningsSection(wikitext);
  const scopedText = [grammarSection, meaningsSection].filter(Boolean).join("\n");
  // Always include full entry as secondary source because many entries put
  // rection markers outside the strict section blocks.
  const caseSearchText = [scopedText, wikitext].filter(Boolean).join("\n");
  if (!caseSearchText) {
    return [];
  }
  const searchableText = toSearchableText(caseSearchText);

  const details = [];
  const pushDetail = (value) => {
    const normalized = normalizeGovernanceDetail(value);
    if (!normalized) {
      return;
    }
    if (!details.includes(normalized)) {
      details.push(normalized);
    }
  };

  const simpleCaseRegex = /mit\s+(Akkusativ|Dativ|Genitiv)\b/gi;
  let match;
  while ((match = simpleCaseRegex.exec(searchableText)) !== null) {
    pushDetail(`mit ${match[1]}`);
  }

  const combinedCaseRegex =
    /mit\s+(Akkusativ|Dativ|Genitiv)\s+und\s+(Akkusativ|Dativ|Genitiv)\b/gi;
  while ((match = combinedCaseRegex.exec(searchableText)) !== null) {
    pushDetail(`mit ${match[1]} und ${match[2]}`);
  }

  const markerRegex = /jemande[mnrs]?\s*\((Dativ|Akkusativ|Genitiv)\)/gi;
  while ((match = markerRegex.exec(searchableText)) !== null) {
    pushDetail(`Marker: ${match[1]}`);
  }

  const markerShortRegex = /jemande[mnrs]?\s*\((Dat\.|Akk\.|Gen\.)\)/gi;
  while ((match = markerShortRegex.exec(searchableText)) !== null) {
    pushDetail(`Marker: ${match[1]}`);
  }

  const prepCaseRegex =
    /mit\s+([a-zäöüß]{1,18})\s*\+\s*(Akkusativ|Dativ|Genitiv)\b/gi;
  while ((match = prepCaseRegex.exec(searchableText)) !== null) {
    pushDetail(`${match[1]} + ${match[2]}`);
  }

  const prepCaseShortRegex =
    /(?:mit\s+)?([a-zäöüß]{1,18})\s*\+\s*(Akk\.|Dat\.|Gen\.)\b/gi;
  while ((match = prepCaseShortRegex.exec(searchableText)) !== null) {
    pushDetail(`${match[1]} + ${match[2]}`);
  }

  const objectRegex = /(Akkusativobjekt|Dativobjekt|Genitivobjekt)/gi;
  while ((match = objectRegex.exec(searchableText)) !== null) {
    const raw = match[1];
    if (/akkusativ/i.test(raw)) {
      pushDetail("Akkusativobjekt");
    } else if (/dativ/i.test(raw)) {
      pushDetail("Dativobjekt");
    } else if (/genitiv/i.test(raw)) {
      pushDetail("Genitivobjekt");
    }
  }

  // Generic abbreviated case markers that frequently appear in verb rection lines.
  const genericCaseRegex = /(?:\(|\b)(Akk\.|Dat\.|Gen\.)(?:\)|\b)/gi;
  while ((match = genericCaseRegex.exec(searchableText)) !== null) {
    pushDetail(`Marker: ${match[1]}`);
  }

  // Common dictionary shorthand: "etw. Akk.", "jdn. Akk.", "jdm. Dat."
  const shorthandRegex = /\b(etw\.|jdn\.|jdm\.)\s*(Akk\.|Dat\.|Gen\.)\b/gi;
  while ((match = shorthandRegex.exec(searchableText)) !== null) {
    pushDetail(`${match[1]} ${match[2]}`);
  }

  // Valency shorthand without explicit case suffixes.
  if (/\bjdm\./i.test(searchableText)) {
    pushDetail("jdm. (Dativ)");
  }
  if (/\bjdn\./i.test(searchableText)) {
    pushDetail("jdn. (Akkusativ)");
  }
  if (/\bjmdn\./i.test(searchableText)) {
    pushDetail("jmdn. (Akkusativ)");
  }

  // Preposition + shorthand valency (e.g. "bei jdm.", "auf jdn.")
  const prepValencyRegex = /\b([a-zäöüß]{2,16})\s+(jdm\.|jdn\.|jmdn\.)\b/gi;
  while ((match = prepValencyRegex.exec(searchableText)) !== null) {
    const prep = match[1];
    const target = match[2].toLowerCase();
    if (target === "jdm.") {
      pushDetail(`${prep} + Dativ`);
    } else {
      pushDetail(`${prep} + Akkusativ`);
    }
  }

  return details;
}

function summarizeCaseGovernance(details) {
  if (!details.length) {
    return UNKNOWN;
  }

  const text = details.join(" ").toLowerCase();
  const hasAkk = text.includes("akkusativ") || text.includes("akk.");
  const hasDat = text.includes("dativ") || text.includes("dat.");

  if (hasAkk && hasDat) {
    return "Akkusativ+Dativ";
  }
  if (hasAkk) {
    return "Akkusativ";
  }
  if (hasDat) {
    return "Dativ";
  }
  return UNKNOWN;
}

function escapeRegex(value) {
  return String(value).replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

function detectReflexiveInfo(wikitext, infinitive = "") {
  const grammarSection = extractGrammarSection(wikitext);
  const meaningsSection = extractMeaningsSection(wikitext);
  const scopedText = [grammarSection, meaningsSection].filter(Boolean).join("\n");
  const text = [scopedText, wikitext].filter(Boolean).join("\n");
  if (!text) {
    return { isReflexive: false, reflexiveDetails: [] };
  }
  const searchableText = toSearchableText(text);

  const details = [];
  const push = (value) => {
    const cleaned = value.replace(/\s+/g, " ").trim();
    if (cleaned && !details.includes(cleaned)) {
      details.push(cleaned);
    }
  };

  if (/\breflexiv\b/i.test(searchableText)) {
    push("reflexiv");
  }
  if (/Wortart\|reflexives?\s+Verb\|Deutsch/i.test(text)) {
    push("Wortart: reflexives Verb");
  }
  if (/{{\s*refl\.?\s*}}|{{\s*refl\|/i.test(text)) {
    push("refl. marker");
  }
  if (infinitive) {
    const infinitivePattern = new RegExp(
      `\\bsich\\b[^\\n]{0,40}\\b${escapeRegex(infinitive)}\\b`,
      "i"
    );
    if (infinitivePattern.test(searchableText)) {
      push(`sich ${infinitive}`);
    }
  }

  // Valency shorthand like "jdn./sich", "jdm./sich"
  if (/jdn\.\s*\/\s*sich|jdm\.\s*\/\s*sich|sich\s*\/\s*jdn\.|sich\s*\/\s*jdm\./i.test(searchableText)) {
    push("valency: /sich");
  }

  return { isReflexive: details.length > 0, reflexiveDetails: details };
}

export function extractVerbInfo(wikitext, lemmaTitle = "") {
  const hasVerbMarker =
    /{{\s*Wortart\|Verb\|Deutsch/i.test(wikitext) ||
    /Deutsch Verb Übersicht/i.test(wikitext);
  if (!hasVerbMarker) {
    return undefined;
  }

  const fieldMap = buildFieldMap(wikitext);

  let present = pickFirstFieldFromMap(fieldMap, [
    "Infinitiv",
    "Infinitiv Präsens",
    "Infinitiv I",
    "Grundform"
  ]);
  if (isUnknown(present) && lemmaTitle) {
    present = cleanupValue(lemmaTitle);
  }

  const presentConjugation = extractConjugation(fieldMap, {
    ich: ["Präsens_ich", "Praesens_ich"],
    du: ["Präsens_du", "Praesens_du"],
    "er/sie/es": [
      "Präsens_er, sie, es",
      "Präsens_er_sie_es",
      "Praesens_er,sie,es",
      "Praesens_er_sie_es"
    ],
    wir: [
      "Präsens_wir",
      "Praesens_wir",
      "Präsens_wir, sie",
      "Präsens_wir_sie",
      "Praesens_wir,sie",
      "Praesens_wir_sie"
    ],
    ihr: ["Präsens_ihr", "Praesens_ihr"],
    sie: [
      "Präsens_sie",
      "Praesens_sie",
      "Präsens_wir, sie",
      "Präsens_wir_sie",
      "Praesens_wir,sie",
      "Praesens_wir_sie"
    ]
  });

  const preteriteConjugation = extractConjugation(fieldMap, {
    ich: ["Präteritum_ich", "Praeteritum_ich"],
    du: ["Präteritum_du", "Praeteritum_du"],
    "er/sie/es": [
      "Präteritum_er, sie, es",
      "Präteritum_er_sie_es",
      "Praeteritum_er,sie,es",
      "Praeteritum_er_sie_es"
    ],
    wir: [
      "Präteritum_wir",
      "Praeteritum_wir",
      "Präteritum_wir, sie",
      "Präteritum_wir_sie",
      "Praeteritum_wir,sie",
      "Praeteritum_wir_sie"
    ],
    ihr: ["Präteritum_ihr", "Praeteritum_ihr"],
    sie: [
      "Präteritum_sie",
      "Praeteritum_sie",
      "Präteritum_wir, sie",
      "Präteritum_wir_sie",
      "Praeteritum_wir,sie",
      "Praeteritum_wir_sie"
    ]
  });

  const pastParticiple = pickFirstFieldFromMap(fieldMap, [
    "Partizip II",
    "Partizip Perfekt"
  ]);

  const auxiliaryVerbs = extractAuxiliaryVerbs(fieldMap);
  const perfectConjugation = buildPerfectConjugation(auxiliaryVerbs, pastParticiple);
  const caseGovernanceDetails = extractCaseGovernanceDetails(wikitext);
  const caseGovernanceSummary = summarizeCaseGovernance(caseGovernanceDetails);
  const reflexiveInfo = detectReflexiveInfo(wikitext, present);
  const completedPresentConjugation = fillPresentFallback(
    presentConjugation,
    present
  );
  const completedPreteriteConjugation = fillPreteriteFallback(
    preteriteConjugation,
    present
  );

  const infinitiveForms = [present];
  if (reflexiveInfo.isReflexive && !isUnknown(present)) {
    infinitiveForms.push(`sich ${present}`);
  }

  return {
    present,
    preterite: pickFirstFieldFromMap(fieldMap, [
      "Präteritum_ich",
      "Präteritum_er, sie, es",
      "Praeteritum_ich",
      "Praeteritum_er,sie,es",
      "Präteritum"
    ]),
    pastParticiple,
    caseGovernance: caseGovernanceSummary,
    caseGovernanceSummary,
    caseGovernanceDetails,
    infinitiveForms: dedupe(infinitiveForms),
    reflexive: reflexiveInfo.isReflexive ? "Yes" : "Unknown",
    reflexiveDetails: reflexiveInfo.reflexiveDetails,
    presentConjugation: completedPresentConjugation,
    preteriteConjugation: completedPreteriteConjugation,
    auxiliaryVerbs,
    perfectConjugation
  };
}

export function extractLemmaCandidate(wikitext, title) {
  const grammarSection = extractGrammarSection(wikitext);
  const searchZone = grammarSection || wikitext;

  const patterns = [
    /{{\s*Grundformverweis[^|}]*\|([^|}\n]+)[^}]*}}/i,
    /{{\s*Form von\|([^|}\n]+)[^}]*}}/i,
    /Grundform:\s*\[\[([^\]|]+)(?:\|[^\]]+)?\]\]/i,
    /Plural(?:form)?(?:\s+des\s+Substantivs)?\s*\[\[([^\]|]+)(?:\|[^\]]+)?\]\]/i,
    /Nominativ\s+Plural\s+des\s+Substantivs\s*\[\[([^\]|]+)(?:\|[^\]]+)?\]\]/i
  ];

  for (const pattern of patterns) {
    const match = searchZone.match(pattern);
    if (match) {
      const lemma = cleanupValue(match[1]);
      if (
        !isUnknown(lemma) &&
        lemma.toLowerCase() !== title.toLowerCase()
      ) {
        return lemma;
      }
    }
  }
  return null;
}

export function hasInflectedFormMarkers(wikitext) {
  const grammarSection = extractGrammarSection(wikitext) || "";
  const zone = grammarSection || wikitext;
  return /Grundformverweis|Form von|Grundform:|flektierte Form|Nominativ\s+Plural\s+des\s+Substantivs|Pluralform\s+des\s+Substantivs/i.test(
    zone
  );
}

function mergeUnknownFields(primary, fallback) {
  if (!primary && fallback) {
    return fallback;
  }
  if (!primary || !fallback) {
    return primary;
  }

  const merged = { ...primary };
  for (const [key, value] of Object.entries(fallback)) {
    if (isUnknown(merged[key]) && !isUnknown(value)) {
      merged[key] = value;
    }
  }
  return merged;
}

export function parseEntry({
  sourceWord,
  title,
  wikitext,
  fallback = null
}) {
  const partOfSpeech = detectPartOfSpeech(wikitext);
  const translations = extractTranslations(wikitext);
  const nounInfo = mergeUnknownFields(extractNounInfo(wikitext), fallback?.nounInfo);
  const verbInfo = mergeUnknownFields(
    extractVerbInfo(wikitext, title),
    fallback?.verbInfo
  );

  const finalTranslations =
    translations.length > 0 ? translations : fallback?.translations || [];

  const notes = [];
  if (finalTranslations.length === 0) {
    notes.push("No English translations were extracted from this entry.");
  }

  return {
    sourceWord,
    normalizedWord: title,
    title,
    partOfSpeech,
    translations: finalTranslations,
    nounInfo,
    verbInfo,
    notes
  };
}
