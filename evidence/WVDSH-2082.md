# WVDSH-2082 validation evidence

`wvdsh-2082-sdk-defold-ab.mp4` is a deterministic A/B validation recorded from
two Defold 1.12.4 HTML5 builds running through `wavedash dev`.

- Left: sdk-defold 1.1.0 (`e9db2baa`) throws WebAssembly
  `memory access out of bounds` after the async Lua coroutine yields and its last
  game-owned reference is removed before forced garbage collection.
- Right: PR #9 (`d39f9621`) retains and routes the coroutine until the Promise
  settles, so the same operation resumes and passes.

The projects and test script are otherwise identical.

MP4 SHA-256:
`83e6e5ef55dd242a237d97f4856cb96f40ab4975734d7ddcc09c6cfd56166979`
