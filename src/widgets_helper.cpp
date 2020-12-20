#include "widgets_helper.hpp"

namespace WidgetsHelper {
	wxSizer* GetSizerItemLabel(wxWindow* parent, wxControl* control, const wxString& label) {
		wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		wxStaticText* text = new wxStaticText(parent, wxID_ANY, label);
		
		sizer->Add(text, 0, wxALL | wxGROW, 0);
		sizer->Add(control, 0, wxALL | wxGROW, 0);
		return sizer;
	};
	
	wxSizer* JoinWidgetsOnSizerH(wxSizer* control1, wxSizer* control2, const int& spaceBetween) {
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(control1, 1, wxRIGHT | wxGROW, spaceBetween);
		sizer->Add(control2, 1, wxALL | wxGROW, 0);
		return sizer;
	};
	
	wxSizer* JoinWidgetsOnSizerH(wxControl* control1, wxControl* control2, const int& spaceBetween) {
		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(control1, 1, wxRIGHT | wxGROW, spaceBetween);
		sizer->Add(control2, 1, wxALL | wxGROW, 0);
		return sizer;
	};
	
	wxSizer* JoinWidgetsOnSizerV(wxSizer* control1, wxSizer* control2, const int& spaceBetween) {
		wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(control1, 1, wxBOTTOM | wxGROW, spaceBetween);
		sizer->Add(control2, 1, wxGROW);
		return sizer;
	}
	
	wxSizer* JoinWidgetsOnSizerV(wxControl* control1, wxControl* control2, const int& spaceBetween) {
		wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(control1, 1, wxBOTTOM | wxGROW, spaceBetween);
		sizer->Add(control2, 1, wxGROW);
		return sizer;
	}
}