# German Dictionary Chrome Extension

Chrome MV3 extension for German -> English lookup using German Wiktionary.

## Features

- Auto-lookup when a German word is highlighted before opening popup.
- Manual lookup with top search box.
- Noun details: article (`der/die/das`) and plural.
- Verb details: present, past tense (preterite), past participle, and explicit case governance (`Akkusativ`, `Dativ`, or both).
- Best-effort parsing with `Unknown` for missing fields.

## Load in Chrome

1. Open `chrome://extensions`.
2. Enable **Developer mode**.
3. Click **Load unpacked**.
4. Select `/Users/niuklear/Documents/Portfolio/DictAPI/german-dictionary-extension`.

## Usage

1. Highlight a German word on any webpage.
2. Click the extension icon.
3. Popup auto-runs translation for the highlighted word.
4. Use the search box for follow-up lookups.

## Notes

- Source: `https://de.wiktionary.org/w/api.php`.
- Parsing is wikitext-based, so some entries may not expose all grammar fields.
- Inflected verbs are resolved to lemma when a detectable lemma reference exists.

## Quick test words

- Noun: `Haus`
- Verb lemma: `gehen`
- Verb inflected form: `ging`
