#include <iostream>
#include "widgets_helper_templates.hpp"

#include "wx/stattext.h"
#include "wx/window.h"
#include "wx/sizer.h"
#include "wx/control.h"
#include "wx/string.h"

namespace WidgetsHelper {
	
	template <typename T, typename R>
	wxSizer* JoinWidgetsOnSizerH(T* control1, R* control2, const int& spaceBetween) {
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(control1, 1, wxRIGHT | wxGROW, spaceBetween);
		sizer->Add(control2, 1, wxALL | wxGROW, 0);
		return sizer;
	};
		
	template <typename T, typename R>
	wxSizer* JoinWidgetsOnSizerV(T* control1, R* control2, const int& spaceBetween) {
		wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(control1, 1, wxBOTTOM | wxGROW, spaceBetween);
		sizer->Add(control2, 1, wxGROW);
		return sizer;
	}
}