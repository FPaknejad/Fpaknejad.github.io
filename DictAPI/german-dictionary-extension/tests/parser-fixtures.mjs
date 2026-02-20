export const fixtures = [
  {
    name: "noun_haus",
    sourceWord: "Haus",
    title: "Haus",
    wikitext: `
== Haus (Deutsch) ==
{{Wortart|Substantiv|Deutsch}}
{{Deutsch Substantiv Übersicht
|Genus=n
|Nominativ Singular=das Haus
|Nominativ Plural=Häuser
}}
=== Übersetzungen ===
* {{Ü|en|house}}
`,
    assert(result, helpers) {
      helpers.expectEqual(result.partOfSpeech, "Noun", "partOfSpeech");
      helpers.expectEqual(result.nounInfo?.article, "das", "noun article");
      helpers.expectEqual(result.nounInfo?.plural, "Häuser", "noun plural");
      helpers.expectIncludes(result.translations, "house", "translations");
    }
  },
  {
    name: "verb_gehen",
    sourceWord: "gehen",
    title: "gehen",
    wikitext: `
== gehen (Deutsch) ==
{{Wortart|Verb|Deutsch}}
{{Deutsch Verb Übersicht
|Infinitiv=gehen
|Präsens_ich=gehe
|Präsens_du=gehst
|Präsens_er, sie, es=geht
|Präsens_wir, sie=gehen
|Präsens_ihr=geht
|Präteritum_ich=ging
|Präteritum_du=gingst
|Präteritum_er, sie, es=ging
|Präteritum_wir, sie=gingen
|Präteritum_ihr=gingt
|Partizip II=gegangen
|Hilfsverb_1=sein
}}
=== Übersetzungen ===
* {{Ü|en|go}}
`,
    assert(result, helpers) {
      helpers.expectEqual(result.partOfSpeech, "Verb", "partOfSpeech");
      helpers.expectEqual(result.verbInfo?.present, "gehen", "verb present");
      helpers.expectEqual(result.verbInfo?.preterite, "ging", "verb preterite");
      helpers.expectEqual(
        result.verbInfo?.pastParticiple,
        "gegangen",
        "verb participle"
      );
      helpers.expectIncludes(result.translations, "go", "translations");
      helpers.expectEqual(
        result.verbInfo?.perfectConjugation?.ich,
        "bin gegangen",
        "perfect conjugation ich"
      );
    }
  },
  {
    name: "verb_case_reflexive",
    sourceWord: "erinnern",
    title: "erinnern",
    wikitext: `
== erinnern (Deutsch) ==
{{Wortart|Verb|Deutsch}}
{{Deutsch Verb Übersicht
|Infinitiv=erinnern
|Präteritum_ich=erinnerte
|Partizip II=erinnert
|Hilfsverb_1=haben
}}
=== Bedeutungen ===
* jdn. Akk. an etw. Akk. erinnern
* sich erinnern
=== Grammatische Merkmale ===
* reflexiv
`,
    assert(result, helpers) {
      helpers.expectEqual(result.partOfSpeech, "Verb", "partOfSpeech");
      helpers.expectEqual(
        result.verbInfo?.caseGovernance,
        "Akkusativ",
        "case governance"
      );
      helpers.expectIncludes(
        result.verbInfo?.caseGovernanceDetails || [],
        "jdn. (Akkusativ)",
        "case detail"
      );
      helpers.expectEqual(result.verbInfo?.reflexive, "Yes", "reflexive");
    }
  },
  {
    name: "verb_translation_fallback",
    sourceWord: "laufen",
    title: "laufen",
    wikitext: `
== laufen (Deutsch) ==
{{Wortart|Verb|Deutsch}}
=== Bedeutungen ===
* sich zu Fuß bewegen
=== Other heading ===
* {{Ü|en|run}}
`,
    assert(result, helpers) {
      helpers.expectEqual(result.partOfSpeech, "Verb", "partOfSpeech");
      helpers.expectIncludes(result.translations, "run", "fallback translation");
      helpers.expectEqual(result.notes.length, 0, "notes should be empty when translation exists");
    }
  },
  {
    name: "derived_variants_extract",
    sourceWord: "helfen",
    title: "helfen",
    wikitext: `
== helfen (Deutsch) ==
{{Wortart|Verb|Deutsch}}
=== Abgeleitete Begriffe ===
* [[abhelfen]]
* [[mithelfen]]
* [[helfen]]
* [[Hilfe:Foo]]
`,
    assert(result, helpers) {
      helpers.expectIncludes(
        result.derivedVariants || [],
        "abhelfen",
        "derived includes abhelfen"
      );
      helpers.expectIncludes(
        result.derivedVariants || [],
        "mithelfen",
        "derived includes mithelfen"
      );
      helpers.expectNotIncludes(
        result.derivedVariants || [],
        "helfen",
        "derived excludes self title"
      );
    }
  },
  {
    name: "derived_variants_template_extract",
    sourceWord: "helfen",
    title: "helfen",
    wikitext: `
== helfen (Deutsch) ==
{{Wortart|Verb|Deutsch}}
=== Abgeleitete Begriffe ===
* {{L|de|abhelfen}}, {{L|de|mithelfen}}, {{l+|de|aushelfen}}
* {{Link|de|nachhelfen}}
`,
    assert(result, helpers) {
      helpers.expectIncludes(
        result.derivedVariants || [],
        "abhelfen",
        "template derived includes abhelfen"
      );
      helpers.expectIncludes(
        result.derivedVariants || [],
        "mithelfen",
        "template derived includes mithelfen"
      );
      helpers.expectIncludes(
        result.derivedVariants || [],
        "aushelfen",
        "template derived includes aushelfen"
      );
      helpers.expectIncludes(
        result.derivedVariants || [],
        "nachhelfen",
        "template derived includes nachhelfen"
      );
    }
  },
  {
    name: "derived_variants_unterbegriffe_template",
    sourceWord: "helfen",
    title: "helfen",
    wikitext: `
== helfen (Deutsch) ==
{{Wortart|Verb|Deutsch}}
{{Unterbegriffe}}
:[1] [[abhelfen]], [[aushelfen]], [[mithelfen]]

{{Beispiele}}
:[1] Lass mich dir helfen.
`,
    assert(result, helpers) {
      helpers.expectIncludes(
        result.derivedVariants || [],
        "abhelfen",
        "unterbegriffe includes abhelfen"
      );
      helpers.expectIncludes(
        result.derivedVariants || [],
        "aushelfen",
        "unterbegriffe includes aushelfen"
      );
      helpers.expectIncludes(
        result.derivedVariants || [],
        "mithelfen",
        "unterbegriffe includes mithelfen"
      );
    }
  }
];

export const markerFixture = {
  title: "ging",
  wikitext: `
== ging (Deutsch) ==
{{Wortart|Verb|Deutsch}}
=== Grammatische Merkmale ===
{{Grundformverweis|gehen}}
`
};
