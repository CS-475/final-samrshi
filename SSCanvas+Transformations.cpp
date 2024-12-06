#include "SSCanvas.h"

/// Get the current transformation matrix from the top of the stack.
GMatrix SSCanvas::getCTM() {
    if (matrices.empty()) {
        exit(-1);
    } else {
        return matrices.back();
    }
}

/// Save off a copy of the canvas state (CTM), to be later used if the balancing call to
/// restore() is made. Calls to save/restore can be nested:
/// save();
///     save();
///         concat(...);    // this modifies the CTM
///         .. draw         // these are drawn with the modified CTM
///     restore();          // now the CTM is as it was when the 2nd save() call was made
///     ..
/// restore();              // now the CTM is as it was when the 1st save() call was made
void SSCanvas::save() {
    matrices.push_back(getCTM());
}

/// Copy the canvas state (CTM) that was record in the correspnding call to save() back into
/// the canvas. It is an error to call restore() if there has been no previous call to save().
void SSCanvas::restore() {
    matrices.pop_back();
    if (matrices.empty()) matrices.push_back(GMatrix());
}

/// Modifies the CTM by preconcatenating the specified matrix with the CTM. The canvas
/// is constructed with an identity CTM.
///
/// CTM' = CTM * matrix
void SSCanvas::concat(const GMatrix& matrix) {
    if (matrices.empty()) return;

    GMatrix ctm = std::move(matrices.back());
    matrices.pop_back();
    matrices.push_back(ctm * matrix);
}