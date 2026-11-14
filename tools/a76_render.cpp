// offline render through the shipping engine: a76_render in.f32 out.f32 fs channels key=value...
// keys: input output attack release ratio mix os schpf, m.<model param>; scfile=<f32, same channels> feeds the
// external side-chain (EXT); grfile=<path> writes the gain reduction (dB) per block; block=<n> (default 512).
#include "Engine.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
int main (int argc, char** argv)
{
    if (argc < 5) { std::fprintf (stderr, "usage: a76_render in.f32 out.f32 fs channels [key=value]\n"); return 1; }
    const double fs = std::atof (argv[3]); const int nc = std::atoi (argv[4]);
    axell76::Params p; int os = 2;   // the plugin default (QUALITY High, 4x)
    std::vector<std::pair<std::string, float>> mods;
    std::string scFile, grFile; int B = 512;
    for (int i = 5; i < argc; ++i)
    {
        std::string a = argv[i]; auto e = a.find ('='); if (e == std::string::npos) continue;
        if (a.rfind ("scfile=", 0) == 0) { scFile = a.substr (7); continue; }
        if (a.rfind ("grfile=", 0) == 0) { grFile = a.substr (7); continue; }
        if (a.rfind ("block=", 0) == 0) { B = std::max (16, std::atoi (a.c_str() + 6)); continue; }
        const std::string k = a.substr (0, e); const float v = (float) std::atof (a.c_str() + e + 1);
        if (k == "input") p.inputDb = v; else if (k == "output") p.outputDb = v; else if (k == "attack") p.attack = v;
        else if (k == "release") p.release = v; else if (k == "ratio") p.ratio = (int) v; else if (k == "mix") p.mix = v; else if (k == "os") os = (int) v; else if (k == "schpf") p.scHpfHz = v < 20.0f ? 0.0f : v;
        else if (k.rfind ("m.", 0) == 0) mods.emplace_back (k.substr (2), v);   // research: override one model parameter
    }
    FILE* f = std::fopen (argv[1], "rb"); if (! f) return 2;
    std::vector<float> x; float buf[4096]; size_t r;
    while ((r = std::fread (buf, sizeof (float), 4096, f)) > 0) x.insert (x.end(), buf, buf + r);
    std::fclose (f);
    const int n = (int) x.size() / nc;
    std::vector<float> scx;
    if (! scFile.empty()) { FILE* sf = std::fopen (scFile.c_str(), "rb"); if (! sf) return 4; float t[4096]; size_t q; while ((q = std::fread (t, sizeof (float), 4096, sf)) > 0) scx.insert (scx.end(), t, t + q); std::fclose (sf); p.extSc = true; }
    axell76::Engine eng; eng.setOversamplingLog2 (os);
    if (! mods.empty())
    {
        auto m = eng.modelParams();
        for (auto& [k, v] : mods)
        {
            if (k == "ampDist") m.ampDist = v; else if (k == "fetDist") m.fetDist = v; else if (k == "fetDistIn") m.fetDistIn = v;
            else if (k == "ampOdd") m.ampOdd = v; else if (k == "fetOdd") m.fetOdd = v; else if (k == "relBias") m.relBias = v;
            else if (k == "kneeQuad") m.kneeQuad = v; else if (k == "thump") m.thump = v; else if (k == "thumpHz") m.thumpHz = v; else if (k == "thumpMs") m.thumpMs = v; else if (k == "thumpThr") m.thumpThr = v; else if (k == "thumpCv") m.thumpCv = v; else if (k == "allLagMs") m.allLagMs = v; else if (k == "allAtkSlowMul") m.allAtkSlowMul = v; else if (k == "detBlend") m.detBlend = v; else if (k == "detTauMs") m.detTauMs = v;
            else { std::fprintf (stderr, "unknown model parameter %s\n", k.c_str()); return 3; }
        }
        eng.setModelParams (m);
    } eng.setParams (p, true); eng.prepare (fs, B, nc);
    std::vector<float> ch[2], kc[2]; for (int c = 0; c < nc; ++c) { ch[c].resize ((size_t) B); kc[c].assign ((size_t) B, 0.0f); }
    std::vector<float> gr;
    std::vector<float> y (x.size());
    for (int s = 0; s < n; s += B)
    {
        const int k = std::min (B, n - s);
        for (int c = 0; c < nc; ++c) for (int i = 0; i < k; ++i) ch[c][(size_t) i] = x[(size_t) ((s + i) * nc + c)];
        float* pt[2] = { ch[0].data(), nc > 1 ? ch[1].data() : nullptr };
        if (! scx.empty())
        {
            for (int c = 0; c < nc; ++c) for (int i = 0; i < k; ++i) { const size_t q = (size_t) ((s + i) * nc + c); kc[c][(size_t) i] = q < scx.size() ? scx[q] : 0.0f; }
            const float* kp[2] = { kc[0].data(), nc > 1 ? kc[1].data() : nullptr };
            eng.process (pt, k, kp, nc);
        }
        else eng.process (pt, k);
        gr.push_back (eng.gainReductionDb());
        for (int c = 0; c < nc; ++c) for (int i = 0; i < k; ++i) y[(size_t) ((s + i) * nc + c)] = ch[c][(size_t) i];
    }
    std::printf ("latency %d\n", eng.latencySamples());
    FILE* o = std::fopen (argv[2], "wb"); std::fwrite (y.data(), sizeof (float), y.size(), o); std::fclose (o);
    if (! grFile.empty()) { FILE* g = std::fopen (grFile.c_str(), "wb"); std::fwrite (gr.data(), sizeof (float), gr.size(), g); std::fclose (g); }
    return 0;
}
