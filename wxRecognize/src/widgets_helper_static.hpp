#pragma once

#include "wx/stattext.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/control.h"
#include "wx/string.h"

namespace WidgetsHelper {
	wxSizer* GetSizerItemLabel(wxWindow* parent, wxControl* control, const wxString& label, const wxString& tooltipText = wxEmptyString);
}