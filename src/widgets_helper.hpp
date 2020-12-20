#pragma once
#include "wx/stattext.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/control.h"
#include "wx/string.h"

namespace WidgetsHelper {
	wxSizer* GetSizerItemLabel(wxWindow* parent, wxControl* control, const wxString& label);
	
	wxSizer* JoinWidgetsOnSizerH(wxSizer* control1, wxSizer* control2, const int& spaceBetween = 0);
	wxSizer* JoinWidgetsOnSizerH(wxControl* control1, wxControl* control2, const int& spaceBetween = 0);
	
	wxSizer* JoinWidgetsOnSizerV(wxSizer* control1, wxSizer* control2, const int& spaceBetween = 0);
	wxSizer* JoinWidgetsOnSizerV(wxControl* control1, wxControl* control2, const int& spaceBetween = 0);
}