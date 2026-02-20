import { highlightExampleText } from "../exampleHighlight.js";

let passed = 0;
let failed = 0;

function fail(message) {
  failed += 1;
  console.error(`FAIL: ${message}`);
}

function expectIncludes(text, value, label) {
  if (!String(text).includes(value)) {
    throw new Error(`${label}: expected to include "${value}"`);
  }
}

function expectNotIncludes(text, value, label) {
  if (String(text).includes(value)) {
    throw new Error(`${label}: expected to exclude "${value}"`);
  }
}

const cases = [
  {
    name: "ankommen_split_pair",
    result: {
      sourceWord: "ankommen",
      title: "ankommen",
      partOfSpeech: "Verb",
      verbInfo: {
        presentConjugation: { ich: "komme an" },
        preteriteConjugation: { ich: "kam an" }
      }
    },
    input: "Ich kam gestern in Bremen an.",
    assert(output) {
      expectIncludes(output, "<strong>kam</strong>", "kam bold");
      expectIncludes(output, "<strong>an</strong>", "an bold");
    }
  },
  {
    name: "ankommen_joined_preterite",
    result: {
      sourceWord: "ankommen",
      title: "ankommen",
      partOfSpeech: "Verb",
      verbInfo: {
        presentConjugation: { ich: "komme an" },
        preteriteConjugation: { ich: "kam an" }
      }
    },
    input: "Sie ankam gestern spaet.",
    assert(output) {
      expectIncludes(output, "<strong>ankam</strong>", "ankam bold");
    }
  },
  {
    name: "ankommen_joined_preterite_plural",
    result: {
      sourceWord: "ankommen",
      title: "ankommen",
      partOfSpeech: "Verb",
      verbInfo: {
        presentConjugation: { ich: "komme an" },
        preteriteConjugation: { ich: "kam an" }
      }
    },
    input: "Die Gruppen ankamen sehr spaet.",
    assert(output) {
      expectIncludes(output, "<strong>ankamen</strong>", "ankamen bold");
    }
  },
  {
    name: "ausfahren_zu_infinitive",
    result: {
      sourceWord: "ausfahren",
      title: "ausfahren",
      partOfSpeech: "Verb",
      verbInfo: {
        presentConjugation: { ich: "fahre aus" },
        preteriteConjugation: { ich: "fuhr aus" }
      }
    },
    input: "Er hilft, Menschen auszufahren.",
    assert(output) {
      expectIncludes(output, "<strong>auszufahren</strong>", "auszufahren bold");
    }
  },
  {
    name: "ausfahren_split_pair",
    result: {
      sourceWord: "ausfahren",
      title: "ausfahren",
      partOfSpeech: "Verb",
      verbInfo: {
        presentConjugation: { ich: "fahre aus" },
        preteriteConjugation: { ich: "fuhr aus" }
      }
    },
    input: "Er fuhr sofort aus der Garage aus.",
    assert(output) {
      expectIncludes(output, "<strong>fuhr</strong>", "fuhr bold");
      expectIncludes(output, "<strong>aus</strong>", "aus bold");
    }
  },
  {
    name: "klar_adjective_inflections",
    result: {
      sourceWord: "klar",
      title: "klar",
      partOfSpeech: "Adjective"
    },
    input: "Sie antwortete mit klarer Stimme und klarem Blick.",
    assert(output) {
      expectIncludes(output, "<strong>klarer</strong>", "klarer bold");
      expectIncludes(output, "<strong>klarem</strong>", "klarem bold");
    }
  },
  {
    name: "klar_no_compound_false_positive",
    result: {
      sourceWord: "klar",
      title: "klar",
      partOfSpeech: "Adjective"
    },
    input: "Das ist glasklar und unklares Wetter.",
    assert(output) {
      expectNotIncludes(output, "<strong>glasklar</strong>", "no compound highlight");
      expectNotIncludes(output, "<strong>unklares</strong>", "no prefixed highlight");
    }
  },
  {
    name: "html_escaping_preserved",
    result: {
      sourceWord: "klar",
      title: "klar",
      partOfSpeech: "Adjective"
    },
    input: "<b>klar</b> & klarer",
    assert(output) {
      expectIncludes(output, "&lt;b&gt;<strong>klar</strong>&lt;/b&gt;", "escaped tags");
      expectIncludes(output, "<strong>klarer</strong>", "klarer bold");
    }
  }
];

for (const testCase of cases) {
  try {
    const output = highlightExampleText(testCase.input, testCase.result);
    testCase.assert(output);
    passed += 1;
    console.log(`PASS: ${testCase.name}`);
  } catch (error) {
    fail(`${testCase.name}: ${error.message}`);
  }
}

console.log(`\nSummary: ${passed} passed, ${failed} failed`);
if (failed > 0) {
  process.exitCode = 1;
}
