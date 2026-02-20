import {
  extractLemmaCandidate,
  hasInflectedFormMarkers,
  parseEntry
} from "../parser.js";
import { fixtures, markerFixture } from "./parser-fixtures.mjs";

let passed = 0;
let failed = 0;

function fail(message) {
  failed += 1;
  console.error(`FAIL: ${message}`);
}

const helpers = {
  expectEqual(actual, expected, label) {
    if (actual !== expected) {
      throw new Error(`${label}: expected "${expected}", got "${actual}"`);
    }
  },
  expectIncludes(array, value, label) {
    if (!Array.isArray(array) || !array.includes(value)) {
      throw new Error(`${label}: expected to include "${value}"`);
    }
  },
  expectNotIncludes(array, value, label) {
    if (Array.isArray(array) && array.includes(value)) {
      throw new Error(`${label}: expected to exclude "${value}"`);
    }
  }
};

for (const fixture of fixtures) {
  try {
    const result = parseEntry({
      sourceWord: fixture.sourceWord,
      title: fixture.title,
      wikitext: fixture.wikitext
    });
    fixture.assert(result, helpers);
    passed += 1;
    console.log(`PASS: ${fixture.name}`);
  } catch (error) {
    fail(`${fixture.name}: ${error.message}`);
  }
}

try {
  helpers.expectEqual(
    hasInflectedFormMarkers(markerFixture.wikitext),
    true,
    "hasInflectedFormMarkers"
  );
  helpers.expectEqual(
    extractLemmaCandidate(markerFixture.wikitext, markerFixture.title),
    "gehen",
    "extractLemmaCandidate"
  );
  passed += 1;
  console.log("PASS: inflected_form_marker");
} catch (error) {
  fail(`inflected_form_marker: ${error.message}`);
}

console.log(`\nSummary: ${passed} passed, ${failed} failed`);
if (failed > 0) {
  process.exitCode = 1;
}
