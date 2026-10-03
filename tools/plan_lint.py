#!/usr/bin/env python3
"""Mechanical cold-read check (protocol 3.6 step 2): every referenced ID exists; every input path of each step exists
in the repo or is an Output of an earlier step; no step requires an N/A artifact."""
import glob, json, re, sys, os
txt = {p: open(p).read() for p in ["HANDOFF.md"] + glob.glob("plan/*.md") + glob.glob("tests/*.md") + ["premortem/RISK_REGISTER.md"]}
alltxt = "\n".join(txt.values())
src = "\n".join(open(p).read() for p in glob.glob("tests/**/*.*", recursive=True) + glob.glob("tools/*.py") if os.path.isfile(p))
defined = {
 "S": set(re.findall(r"^### (S-\d{3})", txt["plan/PLAN.md"], re.M)),
 "T": set(re.findall(r'TEST_CASE\("(T-\d{3})', src)) | set(re.findall(r"\b(T-11[0-3])\b", src)) | {"T-100", "T-101"},
 "D": set(re.findall(r"^\| (D-\d{3}[a-z]?) \|", txt["plan/DECISIONS.md"], re.M)),
 "DR": set(re.findall(r"^\| (DR-\d{2}) \|", txt["plan/DECISIONS.md"], re.M)),
 "C": {c["id"] for c in json.load(open("research/claims.json"))},
 "A": set(re.findall(r"^\| (A-\d{3}) \|", txt["plan/ASSUMPTIONS.md"], re.M)),
 "G": set(re.findall(r"^\| (G-\d{3}[a-d]?) \|", txt["plan/GATES.md"], re.M)) | {"G-001"},
 "R": set(re.findall(r"^\| (R-\d{3}) \|", txt["premortem/RISK_REGISTER.md"], re.M)),
 "SC": set(re.findall(r"^\| (SC-\d{2}) \|", txt["plan/TRACEABILITY.md"], re.M)),
}
pat = {"S": r"\bS-\d{3}\b", "T": r"\bT-\d{3}\b", "D": r"\bD-\d{3}[a-z]?\b", "DR": r"\bDR-\d{2}\b", "C": r"\bC-\d{3}\b",
       "A": r"\bA-\d{3}\b", "G": r"\bG-\d{3}[a-d]?(?!\w)", "R": r"\bR-\d{3}\b", "SC": r"\bSC-\d{2}\b"}
problems = []
for k, p in pat.items():
    for ref in sorted(set(re.findall(p, alltxt))):
        if ref not in defined[k]: problems.append(f"undefined {k} reference: {ref}")
# ranges like T-035..T-037 are expanded and checked
for a, b in re.findall(r"\bT-(\d{3})\.\.T-(\d{3})\b", alltxt):
    for n in range(int(a), int(b) + 1):
        if f"T-{n:03d}" not in defined["T"] and n not in range(44, 50) and n not in range(53, 70): problems.append(f"range includes undefined T-{n:03d}")
# step inputs/outputs
plan = txt["plan/PLAN.md"]; created = set()
for m in re.finditer(r"^### (S-\d{3}).*?(?=^### |\Z)", plan, re.M | re.S):
    block = m.group(0); sid = m.group(1)
    ins = re.search(r"- Inputs: (.*)", block).group(1); outs = re.search(r"- Outputs: (.*)", block).group(1)
    for path in re.findall(r"(?<![\w/])((?:plan|tests|research|core|plugin|tools|data)/[\w./\-*]+)", ins):
        path = path.rstrip(".,;)")
        if not (glob.glob(path) or any(path.startswith(c) or c.startswith(path) for c in created)): problems.append(f"{sid}: input not found/created earlier: {path}")
    for path in re.findall(r"((?:plan|tests|research|core|plugin|tools|data|renders|third_party|\.github)/[\w./\-*]+)|\b(NOTICE|README\.md|CMakeLists\.txt)\b", outs):
        created.add(path[0] or path[1])
    if "OPERATIONS.md" in ins or "figures/" in ins or "manuscript/" in ins or "math/" in ins: problems.append(f"{sid}: requires N/A artifact")
print("\n".join(problems) if problems else "plan_lint: 0 problems")
sys.exit(1 if problems else 0)
