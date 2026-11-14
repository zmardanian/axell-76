// Axell 76 2.0's model, frozen exactly as released (2.0.0, rounded to 6 decimals as it shipped): sessions saved before
// 3.0 keep rendering bit-identically. Do not regenerate.
#pragma once
#include "FetModel.h"
namespace axell76
{
inline const ModelParams& fittedParamsV2()
{
    static const ModelParams params = [] {
        ModelParams p;
        const RatioLaw law[5] = {
        { -15.778158f, 0.999951f, -0.008910f, 5.851802f },
        { -16.276219f, 0.959495f, -0.005850f, 3.050984f },
        { -15.129239f, 0.999423f, -0.007015f, 2.627296f },
        { -14.294698f, 0.927840f, -0.000957f, 0.919737f },
        { -16.952759f, 0.999859f, -0.004373f, 1.202469f } };
        for (int i = 0; i < 5; ++i) p.law[i] = law[i];
        p.atkSlowMs = 0.960617f;
        p.atkFastMs = 0.105840f;
        p.atkCurve = -0.856359f;
        p.relSlowMs = 696.197453f;
        p.relFastMs = 19.500638f;
        p.relCurve = 1.450923f;
        p.atkBoost = 0.000000f;
        p.memTauMs = 8832.709184f;
        p.memRel = 0.000000f;
        p.creep = 0.249666f;
        p.allAtkMul = 1.174119f;
        p.allRelMul = 0.087832f;
        p.allGainDb = 1.286434f;
        p.fetDist = 0.000001f;
        p.atkTilt = 0.045316f;
        p.allAtkTilt = 0.069391f;
        p.maxGrDb = 50.466447f;
        p.detAsym = -0.080000f;
        p.ampDist = 0.010392f;
        p.feedback = 0.000000f;
        p.fetDistIn = 0.001504f;
        p.allAtkBoost = 0.000000f;
        p.detBlend = 0.078249f;
        p.detTauMs = 43.099068f;
        return p;
    }();
    return params;
}
} // namespace axell76
