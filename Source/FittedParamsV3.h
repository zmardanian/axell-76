// Axell 76 3.0 / 3.1's model, frozen exactly as released (3.1.0): sessions saved with 3.0 or 3.1 (model mark 3) keep
// rendering bit-identically. Do not regenerate.
#pragma once
#include "FetModel.h"
namespace axell76
{
inline const ModelParams& fittedParamsV3()
{
    static const ModelParams params = [] {
        ModelParams p;
        const RatioLaw law[5] = {
        { -15.7781578f, 0.99995104f, -0.00891020847f, 5.85180199f },
        { -16.2762187f, 0.959494906f, -0.00585000594f, 3.05098354f },
        { -15.1292392f, 0.999422605f, -0.00701521854f, 2.62729616f },
        { -14.2946976f, 0.927839909f, -0.000956915289f, 0.919736746f },
        { -16.9527587f, 0.999858529f, -0.00437275065f, 1.20246935f } };
        for (int i = 0; i < 5; ++i) p.law[i] = law[i];
        p.atkSlowMs = 0.960617414f;
        p.atkFastMs = 0.105839515f;
        p.atkCurve = -0.856359186f;
        p.relSlowMs = 696.197453f;
        p.relFastMs = 19.5006383f;
        p.relCurve = 1.45092252f;
        p.atkBoost = 7.10091854e-08f;
        p.memTauMs = 8832.70918f;
        p.memRel = 2.79774923e-11f;
        p.creep = 0.249666134f;
        p.allAtkMul = 1.17411914f;
        p.allRelMul = 0.0878321907f;
        p.allGainDb = 1.28643444f;
        p.fetDist = 2.18350625e-06f;
        p.atkTilt = 0.0453159195f;
        p.allAtkTilt = 0.0693909237f;
        p.maxGrDb = 50.4664469f;
        p.detAsym = -0.08f;
        p.ampDist = 0.0101028237f;
        p.feedback = 0.0f;
        p.fetDistIn = 0.0016703739f;
        p.allAtkBoost = 9.75531909e-19f;
        p.detBlend = 0.0782487264f;
        p.detTauMs = 43.0990678f;
        p.allLagMs = 0.0f;
        p.allAtkSlowMul = -1.0f;
        p.kneeQuad = 0.0f;
        p.ampOdd = -0.0374920483f;
        p.fetOdd = 0.0306090821f;
        p.relBias = 0.0f;
        p.thump = 0.0f;
        p.thumpHz = 8.0f;
        p.thumpMs = 1.0f;
        p.thumpThr = 0.0f;
        p.thumpCv = -0.0008f;
        return p;
    }();
    return params;
}
} // namespace axell76
