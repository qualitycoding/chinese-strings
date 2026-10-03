#!/usr/bin/env python3
# FROZEN — DO NOT MODIFY. T-112 (D-014): query OSV by pinned commit for every FetchContent dependency.
import json, sys, time, urllib.request
lock = json.load(open(sys.argv[1] if len(sys.argv) > 1 else "plan/deps.lock.json"))
deps = lock["fetchcontent"]
body = json.dumps({"queries": [{"commit": d["commit"]} for d in deps]}).encode()
for attempt in range(3):
    try:
        req = urllib.request.Request("https://api.osv.dev/v1/querybatch", data=body, headers={"Content-Type": "application/json"})
        res = json.load(urllib.request.urlopen(req, timeout=30)); break
    except Exception as e:  # network failure: retry with backoff, never silently pass
        print(f"T-112 attempt {attempt + 1} failed: {e}", file=sys.stderr); time.sleep(5 * 2 ** attempt)
else:
    print("T-112 FAIL: OSV unreachable"); sys.exit(2)
bad = [(d["name"], [v["id"] for v in r.get("vulns", [])]) for d, r in zip(deps, res["results"]) if r.get("vulns")]
if bad: print("T-112 FAIL: vulnerabilities:", bad); sys.exit(1)
print("T-112 PASS:", ", ".join(d["name"] + "@" + d["commit"][:12] for d in deps))
