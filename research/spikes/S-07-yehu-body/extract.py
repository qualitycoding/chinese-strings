"""Spike S-07: extract yehu bridge modal parameters and the 25-section radiation filter from
yehudafx26 (MIT, commit 284e5be25eaf55fdd366b8d9f3917a10da8ee6a4), examples/yehuBridgeAdmittance.mat and
examples/IIR_player.mat. Converts (M,K,R) to (f, T60) using DAFx26 eq. (11): K = M w^2, R = M sigma_m w_m
=> amplitude decay rate = R/(2M), T60 = 3 ln10 / (R/(2M)). Output: yehu_body.json (attribution: MIT, Zheng/Darabundit/Scavone)."""
import json, math, sys, scipy.io as sio
root = sys.argv[1]
m = sio.loadmat(f"{root}/examples/yehuBridgeAdmittance.mat"); M, K, R = (m[k].ravel() for k in ("M", "K", "R"))
modes = []
for Mi, Ki, Ri in zip(M, K, R):
    w = math.sqrt(Ki / Mi); a = Ri / (2 * Mi)
    modes.append({"freqHz": round(w / (2 * math.pi), 3), "t60s": round(3 * math.log(10) / a, 5), "massKg": float(Mi), "stiffnessNm": float(Ki), "dampingKgs": float(Ri)})
iir = sio.loadmat(f"{root}/examples/IIR_player.mat"); Am, Bm = iir["Am"], iir["Bm"]
fir = iir["FIR"].ravel().tolist()
radiation = [{"a": [float(x) for x in Am[:, k]], "b": [float(x) for x in Bm[:, k]]} for k in range(Am.shape[1])]
json.dump({"source": "yehudafx26@284e5be25eaf55fdd366b8d9f3917a10da8ee6a4 (MIT)", "bridgeModes": modes,
           "radiationParallelSections": radiation, "radiationFIR": fir,
           "note": "radiation sections: H(z) = FIR + sum_k (b0 + b1 z^-1)/(a0 + a1 z^-1 + a2 z^-2) per Bank 2018 parallel form; sample rate of design not stored in file"},
          open("yehu_body.json", "w"), indent=1)
for x in modes: print(x["freqHz"], x["t60s"])
print("sections", Am.shape, "FIR", fir)
