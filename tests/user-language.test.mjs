import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import vm from 'node:vm';

const source = readFileSync(
    new URL('../wavedash/lib/web/lib_wavedash.js', import.meta.url),
    'utf8',
).replace(/\{\{\{[\s\S]*?\}\}\}/g, '(function() {})');

for (const [name, window, expected] of [
    ['no browser window', undefined, 'en'],
    ['no host SDK', {}, 'en'],
    ['older SDK without the getter', { Wavedash: {} }, 'en'],
    ['unavailable getter', { Wavedash: { getUserLanguage: null } }, 'en'],
    [
        'unset preference',
        { Wavedash: { getUserLanguage: () => undefined } },
        'en',
    ],
    ['null preference', { Wavedash: { getUserLanguage: () => null } }, 'en'],
    ['empty preference', { Wavedash: { getUserLanguage: () => '' } }, 'en'],
    [
        'invalid preference type',
        { Wavedash: { getUserLanguage: () => 42 } },
        'en',
    ],
    ['saved preference', { Wavedash: { getUserLanguage: () => 'fr' } }, 'fr'],
    [
        'saved regional preference',
        { Wavedash: { getUserLanguage: () => 'pt-BR' } },
        'pt-BR',
    ],
    [
        'getter receiver',
        {
            Wavedash: {
                language: 'zh-TW',
                getUserLanguage() {
                    return this.language;
                },
            },
        },
        'zh-TW',
    ],
]) {
    test(name, () => {
        const context = {
            window,
            autoAddDeps() {},
            addToLibrary() {},
            stringToNewUTF8: (value) => value,
        };
        vm.runInNewContext(source, context);
        context.WavedashJs = context.LibWavedash.$WavedashJs;
        assert.equal(
            context.LibWavedash.WavedashJs_GetUserLanguage(),
            expected,
        );
    });
}
