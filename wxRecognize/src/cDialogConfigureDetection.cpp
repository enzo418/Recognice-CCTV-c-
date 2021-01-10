#include "cDialogConfigureDetection.hpp"

cDialogConfigureDetection::cDialogConfigureDetection( 
							wxWindow * parent, wxWindowID id, const wxString & title,
							const wxPoint & position, const wxSize & size, wxString& yoloCfg, wxString& yoloWeight, wxString& yoloClasses)
							: wxDialog(parent, id, title, position, size, wxDEFAULT_DIALOG_STYLE)
{
	wxString dimensions = "", s;
	wxPoint p;
	wxSize  sz;

	sz.SetWidth(size.GetWidth() - 20);
	sz.SetHeight(size.GetHeight() - 70);

	p.x = 6; p.y = 2;

	dialogText = new wxTextCtrl (this, -1, dimensions, p, sz, wxTE_MULTILINE);

	p.y += sz.GetHeight() + 10;
	wxButton* b = new wxButton(this, wxID_OK, _("OK"), p, wxDefaultSize);
//	b->Bind(wxEVT_BUTTON, &cDialogConfigureDetection::OnOk, this);
	
	p.x += 110;
	wxButton* c = new wxButton( this, wxID_CANCEL, _("Cancel"), p, wxDefaultSize );
}

cDialogConfigureDetection::~cDialogConfigureDetection()
{
}

//void cDialogConfigureDetection::OnOk(wxCommandEvent& ev) {
//	ev.
//}

