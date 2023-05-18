#pragma once

/**
 * TL;DR: Is an interface that is used on an vision library implementation? Then
 * Do NOT include this file. Include IFrame instead.
 *
 * In order to avoid circular implementation you should always check that all
 * the files from implementation don't include the current file! Usually this is
 * the case of the interfaces since an implementation implements an interface.
 *
 * This is why all the interfaces include IFrame and not this file, but all the
 * implementations include this file.
 */

#ifdef USE_OPENCV
#include "../../Implementations/opencv/Implementation.hpp"
#else
#error "No implementation for IFrame"
#endif
