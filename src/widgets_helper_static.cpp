#include "wx/stattext.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/control.h"
#include "wx/string.h"

namespace WidgetsHelper {
	wxSizer* GetSizerItemLabel(wxWindow* parent, wxControl* control, const wxString& label) {
		wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		wxStaticText* text = new wxStaticText(parent, wxID_ANY, label);
		
		sizer->Add(text, 0, wxALL | wxGROW, 0);
		sizer->Add(control, 0, wxALL | wxGROW, 0);
		return sizer;
	};
}