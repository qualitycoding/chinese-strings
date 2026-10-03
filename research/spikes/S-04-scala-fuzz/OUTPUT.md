# S-04 output (2026-10-03T15:41:56Z)

- clang 18.1.3, -fsanitize=fuzzer,address,undefined, tuning-library @ 48422e2f
- Seeds: corpus/et12.scl, corpus/penta.scl (generated corpus not committed)
```
#1747883	REDUCE cov: 551 ft: 1984 corp: 370/53Kb lim: 4096 exec/s: 7313 rss: 524Mb L: 45/1938 MS: 1 EraseBytes-
#1750081	REDUCE cov: 551 ft: 1984 corp: 370/53Kb lim: 4096 exec/s: 7292 rss: 524Mb L: 627/1938 MS: 3 InsertByte-InsertByte-EraseBytes-
#1757001	DONE   cov: 551 ft: 1984 corp: 370/53Kb lim: 4096 exec/s: 7290 rss: 524Mb
Done 1757001 runs in 241 second(s)
```
Result: 1,757,001 executions in 241 s, no crash, no sanitizer report.
