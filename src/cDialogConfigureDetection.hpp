#pragma once
#include "wx/dialog.h"
#include "wx/textctrl.h"
#include "wx/button.h"

class cDialogConfigureDetection : public wxDialog {
	public:
		cDialogConfigureDetection ( wxWindow * parent, wxWindowID id, const wxString & title,
					  const wxPoint & pos,
					  const wxSize & size,
					  wxString& yoloCfg, wxString& yoloWeight, wxString& yoloClasses);
					  
		~cDialogConfigureDetection();
		
		wxTextCtrl * dialogText;
		wxString GetText();

//	private:
//		void OnOk(wxCommandEvent & event);
};

