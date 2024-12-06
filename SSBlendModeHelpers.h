#ifndef SSBlendModeHelpers2_DEFINED
#define SSBlendModeHelpers2_DEFINED

#include "include/GMath.h"
#include "include/GPixel.h"
#include "include/GBlendMode.h"
#include "include/GColor.h"
#include "SSMath.h"

// MARK: Premultiplication

static inline GPixel colorToPixel(const GColor& color) {
    int r = SSRoundPositiveToInt(color.r * color.a * 255);
    int g = SSRoundPositiveToInt(color.g * color.a * 255);
    int b = SSRoundPositiveToInt(color.b * color.a * 255);
    int a = SSRoundPositiveToInt(color.a * 255);

    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel colorToPixelOpaque(const GColor& color) {
    int r = SSRoundPositiveToInt(color.r * 255);
    int g = SSRoundPositiveToInt(color.g * 255);
    int b = SSRoundPositiveToInt(color.b * 255);
    int a = 255;

    return GPixel_PackARGB(a, r, g, b);
}

// MARK: Blend Modes

static inline unsigned divBy255(unsigned n) {
    return (n + 128) * 257 >> 16;
}

static inline auto blendClear = [](GPixel src, GPixel* dst) {
    // 0
    return GPixel_PackARGB(0, 0, 0, 0);
};

static inline auto blendSrc = [](GPixel src, GPixel* dst) {
    // src
    return src;
};

static inline auto blendDst = [](GPixel src, GPixel* dst) {
    // dst
    return *dst;
};

static inline auto blendSrcOver = [](GPixel src, GPixel* dst) {
    auto src_over = [](unsigned src, unsigned src_alpha, unsigned dst) -> unsigned { return src + divBy255((255 - src_alpha) * dst); };

    unsigned src_alpha = GPixel_GetA(src);
    unsigned r = src_over(GPixel_GetR(src), src_alpha, GPixel_GetR(*dst));
    unsigned g = src_over(GPixel_GetG(src), src_alpha, GPixel_GetG(*dst));
    unsigned b = src_over(GPixel_GetB(src), src_alpha, GPixel_GetB(*dst));
    unsigned a = src_over(GPixel_GetA(src), src_alpha, GPixel_GetA(*dst));

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendDstOver = [](GPixel src, GPixel* dst) {
    auto dst_over = [](unsigned src, unsigned dst_alpha, unsigned dst) { return dst + divBy255((255 - dst_alpha) * src); };
    
    unsigned dst_alpha = GPixel_GetA(*dst);
    unsigned r = dst_over(GPixel_GetR(src), dst_alpha, GPixel_GetR(*dst));
    unsigned g = dst_over(GPixel_GetG(src), dst_alpha, GPixel_GetG(*dst));
    unsigned b = dst_over(GPixel_GetB(src), dst_alpha, GPixel_GetB(*dst));
    unsigned a = dst_over(GPixel_GetA(src), dst_alpha, GPixel_GetA(*dst));

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendSrcIn = [](GPixel src, GPixel* dst) {
    auto src_in = [](unsigned src, unsigned dst_alpha) { return divBy255(dst_alpha * src); };

    unsigned dst_alpha = GPixel_GetA(*dst);
    unsigned r = src_in(GPixel_GetR(src), dst_alpha);
    unsigned g = src_in(GPixel_GetG(src), dst_alpha);
    unsigned b = src_in(GPixel_GetB(src), dst_alpha);
    unsigned a = src_in(GPixel_GetA(src), dst_alpha);

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendDstIn = [](GPixel src, GPixel* dst) {
    auto dst_in = [](unsigned src_alpha, unsigned dst) { return divBy255(src_alpha * dst); };

    unsigned src_alpha = GPixel_GetA(src);
    unsigned r = dst_in(src_alpha, GPixel_GetR(*dst));
    unsigned g = dst_in(src_alpha, GPixel_GetG(*dst));
    unsigned b = dst_in(src_alpha, GPixel_GetB(*dst));
    unsigned a = dst_in(src_alpha, GPixel_GetA(*dst));

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendSrcOut = [](GPixel src, GPixel* dst) {
    auto src_out = [](unsigned src, unsigned dst_alpha) { return divBy255((255 - dst_alpha) * src); };

    unsigned dst_alpha = GPixel_GetA(*dst);
    unsigned r = src_out(GPixel_GetR(src), dst_alpha);
    unsigned g = src_out(GPixel_GetG(src), dst_alpha);
    unsigned b = src_out(GPixel_GetB(src), dst_alpha);
    unsigned a = src_out(GPixel_GetA(src), dst_alpha);

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendDstOut = [](GPixel src, GPixel* dst) {
    auto dst_out = [](unsigned src_alpha, unsigned dst) { return divBy255((255 - src_alpha) * dst); };

    unsigned src_alpha = GPixel_GetA(src);
    unsigned r = dst_out(src_alpha, GPixel_GetR(*dst));
    unsigned g = dst_out(src_alpha, GPixel_GetG(*dst));
    unsigned b = dst_out(src_alpha, GPixel_GetB(*dst));
    unsigned a = dst_out(src_alpha, GPixel_GetA(*dst));

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendSrcATop = [](GPixel src, GPixel* dst) {
    auto src_a_top = [](unsigned src, unsigned src_alpha, unsigned dst, unsigned dst_alpha) { return divBy255((dst_alpha * src) + (255 - src_alpha) * dst); };

    unsigned src_alpha = GPixel_GetA(src);
    unsigned dst_alpha = GPixel_GetA(*dst);
    unsigned r = src_a_top(GPixel_GetR(src), src_alpha, GPixel_GetR(*dst), dst_alpha);
    unsigned g = src_a_top(GPixel_GetG(src), src_alpha, GPixel_GetG(*dst), dst_alpha);
    unsigned b = src_a_top(GPixel_GetB(src), src_alpha, GPixel_GetB(*dst), dst_alpha);
    unsigned a = src_a_top(GPixel_GetA(src), src_alpha, GPixel_GetA(*dst), dst_alpha);

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendDstATop = [](GPixel src, GPixel* dst) {
    auto dst_a_top = [](unsigned src, unsigned src_alpha, unsigned dst, unsigned dst_alpha) { return divBy255((src_alpha * dst) + (255 - dst_alpha) * src); };

    unsigned src_alpha = GPixel_GetA(src);
    unsigned dst_alpha = GPixel_GetA(*dst);
    unsigned r = dst_a_top(GPixel_GetR(src), src_alpha, GPixel_GetR(*dst), dst_alpha);
    unsigned g = dst_a_top(GPixel_GetG(src), src_alpha, GPixel_GetG(*dst), dst_alpha);
    unsigned b = dst_a_top(GPixel_GetB(src), src_alpha, GPixel_GetB(*dst), dst_alpha);
    unsigned a = dst_a_top(GPixel_GetA(src), src_alpha, GPixel_GetA(*dst), dst_alpha);

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendXor = [](GPixel src, GPixel* dst) {
    auto blend_xor = [](unsigned src, unsigned src_alpha, unsigned dst, unsigned dst_alpha) { return divBy255((255 - src_alpha) * dst + (255 - dst_alpha) * src); };

    unsigned src_alpha = GPixel_GetA(src);
    unsigned dst_alpha = GPixel_GetA(*dst);
    unsigned r = blend_xor(GPixel_GetR(src), src_alpha, GPixel_GetR(*dst), dst_alpha);
    unsigned g = blend_xor(GPixel_GetG(src), src_alpha, GPixel_GetG(*dst), dst_alpha);
    unsigned b = blend_xor(GPixel_GetB(src), src_alpha, GPixel_GetB(*dst), dst_alpha);
    unsigned a = blend_xor(GPixel_GetA(src), src_alpha, GPixel_GetA(*dst), dst_alpha);

    return GPixel_PackARGB(a, r, g, b);
};

static inline auto blendModulate = [](GPixel src, GPixel* dst) {
    auto blend_modulate = [](unsigned src, unsigned dst) { return divBy255(src * dst); };

    unsigned r = blend_modulate(GPixel_GetR(src), GPixel_GetR(*dst));
    unsigned g = blend_modulate(GPixel_GetG(src), GPixel_GetG(*dst));
    unsigned b = blend_modulate(GPixel_GetB(src), GPixel_GetB(*dst));
    unsigned a = blend_modulate(GPixel_GetA(src), GPixel_GetA(*dst));

    return GPixel_PackARGB(a, r, g, b);
};

/// MARK: Blend Mode Simplification

static inline GBlendMode simplifyBlendMode(GBlendMode original, bool isOpaque, bool isTransparent) {
    // src simplification
    if (original == GBlendMode::kSrc) {
        // if (src.a == 0) {
        if (isTransparent) {
            return GBlendMode::kClear;
        }
    }
    // srcOver simplification
    else if (original == GBlendMode::kSrcOver) {
        if (isOpaque) {
            return GBlendMode::kSrc;
        } else if (isTransparent) {
            return GBlendMode::kDst;
        }
    }
    // dstOver simplification
    else if (original == GBlendMode::kDstOver) {
        if (isTransparent) {
            return GBlendMode::kDst;
        }
    }
    // srcIn simplification
    else if (original == GBlendMode::kSrcIn) {
        if (isTransparent) {
            return GBlendMode::kClear;
        }
    }
    // dstIn simplification
    else if (original == GBlendMode::kDstIn) {
        if (isOpaque) {
            return GBlendMode::kDst;
        } else if (isTransparent) {
            return GBlendMode::kClear;
        }
    }
    // dstOut simplifications
    else if (original == GBlendMode::kSrcOut) {
        if (isTransparent) {
            return GBlendMode::kClear;
        }
    }
    // dstOut simplifications
    else if (original == GBlendMode::kDstOut) {
        if (isOpaque) {
            return GBlendMode::kClear;
        } else if (isTransparent) {
            return GBlendMode::kDst;
        }
    }
    // srcATop simplifications
    else if (original == GBlendMode::kSrcATop) {
        if (isOpaque) {
            return GBlendMode::kSrcIn;
        } else if (isTransparent) {
            return GBlendMode::kDst;
        }
    }
    // dstATop simplifications
    else if (original == GBlendMode::kDstATop) {
        if (isOpaque) {
            return GBlendMode::kDstOver;
        } else if (isTransparent) {
            return GBlendMode::kClear;
        }
    }
    // xor simplifications
    else if (original == GBlendMode::kXor) {
        if (isOpaque) {
            return GBlendMode::kSrcOut;
        } else if (isTransparent) {
            return GBlendMode::kDst;
        }
    }
    
    // Return original if none of the simplifications were hit
    return original;
}

#endif // SSBlendModeHelpers2_DEFINED