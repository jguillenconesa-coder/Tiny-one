// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "SampleData.h"
namespace pocket {
// Authored offline preparation and initial equal regions, informed by manual
// section 6.2. This is not a recovered segmentation/import algorithm.
inline bool prepareSharedDrum246(Sample& sample) {
 if(sample.frames.empty()||!std::isfinite(sample.rate)||sample.rate<8000||sample.rate>192000)return false;
 const auto limit=size_t(sample.rate*12.);if(sample.frames.size()>limit)sample.frames.resize(limit);
 for(auto& value:sample.frames)value=std::isfinite(value)?std::clamp(value,-1.f,1.f):0.f;
 sample.prepareDrumPcm();return !sample.drumPcm.empty();
}
inline PadSettings sharedDrum246Region(int pad) noexcept {
 PadSettings p;pad=std::clamp(pad,0,23);p.start=float(pad)/24;p.end=float(pad+1)/24;return p;
}
}
