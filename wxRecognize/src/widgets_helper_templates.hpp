#ifndef _TESTTEMP_H_
#define _TESTTEMP_H_

#include "wx/stattext.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/control.h"
#include "wx/string.h"

namespace WidgetsHelper {
	
	template <typename T, typename R>
	wxSizer* JoinWidgetsOnSizerH(T* control1, R* control2, const int& spaceBetween = 0);
	
	template <typename T, typename R>
	wxSizer* JoinWidgetsOnSizerV(T* control1, R* control2, const int& spaceBetween = 0);
}

#include "widgets_helper_templates.tpp"

#endif