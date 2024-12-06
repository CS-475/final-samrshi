#include "SSFinal.h"

/// Implement this to return ain instance of your subclass of GFinal.
std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<SSFinal>(new SSFinal());
}