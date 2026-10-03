// Spike S-05: (a) cost of a representative DWG voice (2 strings x [delay + 4 dispersion allpasses + loss filter]
// + 24-mode modal body + 25-section parallel radiation filter), 16 voices, 48 kHz, 128-sample blocks;
// (b) audio-thread allocation detection via global operator new counter (test-binary-only technique);
// (c) calibration loop to normalise CPU speed across machines (D-008 candidate).
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <vector>
static std::atomic<long> gAllocs{0}; static thread_local bool gGuard = false;
void* operator new(std::size_t n) { if (gGuard) gAllocs++; if (void* p = std::malloc(n)) return p; throw std::bad_alloc(); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
struct Biquad { float b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0; float tick(float x){ float y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y; return y; } };
struct AP1 { float a=-0.3f, x1=0, y1=0; float tick(float x){ float y=a*x+x1-a*y1; x1=x; y1=y; return y; } };
struct String { std::vector<float> d; int i=0; AP1 disp[4]; float lp=0;
  explicit String(int n):d((size_t)n,0.f){ for(int k=0;k<n;++k) d[(size_t)k]=std::sin(0.37f*k); }
  float tick(float in){ float y=d[(size_t)i]; float v=y; for(auto& a:disp) v=a.tick(v); lp=0.5f*(v+lp); d[(size_t)i]=0.996f*lp+in; i=(i+1)%(int)d.size(); return y; } };
struct Voice { String s1{163}, s2{109}; Biquad body[24], rad[25]; float fb=0;
  Voice(){ for(int m=0;m<24;++m){ float w=2*3.14159f*(200.f+150.f*m)/48000.f, r=0.999f; body[m].a1=-2*r*std::cos(w); body[m].a2=r*r; body[m].b0=0.001f; }
           for(int m=0;m<25;++m){ float w=2*3.14159f*(100.f+200.f*m)/48000.f, r=0.995f; rad[m].a1=-2*r*std::cos(w); rad[m].a2=r*r; rad[m].b0=0.01f; } }
  float tick(float bow){ float f=s1.tick(bow*0.01f+fb)+s2.tick(0); float b=0; for(auto& q:body) b+=q.tick(f); fb=-1e-5f*b; float o=0; for(auto& q:rad) o+=q.tick(f); return o; } };
int main(){
  const int fs=48000, block=128, voices=16, seconds=10; std::vector<Voice> v((size_t)voices); std::vector<float> out((size_t)block);
  // calibration: fixed scalar workload
  auto c0=std::chrono::steady_clock::now(); volatile double acc=0; for(long k=0;k<200000000L;++k) acc+=1e-9*(double)(k&1023);
  double calib=std::chrono::duration<double>(std::chrono::steady_clock::now()-c0).count();
  gGuard=true; auto t0=std::chrono::steady_clock::now();
  for(int b=0;b<fs*seconds/block;++b){ for(int n=0;n<block;++n){ float s=0; for(auto& x:v) s+=x.tick(0.1f); out[(size_t)n]=s; } }
  double el=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count(); gGuard=false;
  if(!std::isfinite(out[0])){std::printf("UNSTABLE\n");return 2;}
  std::printf("calibration_s=%.3f  render_s=%.3f for %d s audio, %d voices -> core_load=%.1f%%  normalised_load=%.3f  audio_thread_allocs=%ld  (out=%g)\n",
    calib, el, seconds, voices, 100.0*el/seconds, (el/seconds)/calib, gAllocs.load(), (double)out[0]);
  // (b) negative control: an allocation inside the guarded region must be detected
  gGuard=true; auto* p=new std::vector<float>(64); gGuard=false; delete p; std::printf("negative_control_allocs=%ld (expect >=1 more)\n", gAllocs.load());
}
