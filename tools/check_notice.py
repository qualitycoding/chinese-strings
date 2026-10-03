#!/usr/bin/env python3
# FROZEN — DO NOT MODIFY. T-113 (D-002): LICENSE is Apache-2.0; NOTICE names every dependency and its licence; README states AGPLv3 binary terms.
import json, sys
ok = True
lic = open("LICENSE").read(); notice = open("NOTICE").read() if __import__("os").path.exists("NOTICE") else ""; readme = open("README.md").read()
if "Apache License" not in lic or "Version 2.0" not in lic: print("LICENSE is not Apache-2.0"); ok = False
for d in json.load(open("plan/deps.lock.json"))["fetchcontent"]:
    if d["name"] not in notice: print("NOTICE missing", d["name"]); ok = False
for word in ["AGPL", "JUCE", "VST3", "MIT", "BSL-1.0"]:
    if word not in notice: print("NOTICE missing", word); ok = False
if "AGPLv3" not in readme: print("README missing AGPLv3 binary statement"); ok = False
print("T-113 PASS" if ok else "T-113 FAIL"); sys.exit(0 if ok else 1)
