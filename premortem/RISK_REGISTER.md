# Risk Register

| ID | Description | Lens | Severity | Likelihood | Root cause | Traces to | Mitigation | Status |
|---|---|---|---|---|---|---|---|---|
| R-001 | Erhu-family membrane/body defaults are not grounded in a direct measurement (skin-mode values contradict across sources) → timbre may be off. | invalid research assumptions | Medium | Medium | Only computed (C-033) or Tier 3 (C-008) values exist | C-008, C-029, C-033 | Body modes are data-table defaults (D-011); human listening gate G-003a; tests never assert contested values | open (Phase 4) |
| R-002 | Plucked-lute body/radiation defaults rest on single sources (pipa/yueqin/ruan). | invalid research assumptions | Medium | Medium | Few published measurements | C-014, C-015, C-017 | Same as R-001; gate G-003b | open (Phase 4) |
| R-003 | Yangqin bridge-impedance shape comes from one thesis. | invalid research assumptions | Medium | Low | Single author | C-016 | Data-table default; gate G-003d | open (Phase 4) |
| R-004 | Dropped instruments (Tier C, gehu, zhuihu, qinqin, leiqin) may have literature not found in English/Chinese web search. | invalid research assumptions | Medium | Medium | Search coverage | C-044 | Documented drop list in README; re-addable by future plan | open (Phase 4) |
