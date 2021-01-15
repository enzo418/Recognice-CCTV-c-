#pragma once

#include <wx/log.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/bookctrl.h>
#include <wx/frame.h>
#include "wx/textctrl.h"
#include <wx/thread.h>
#include <wx/event.h>

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#ifdef _WIN32 
#include <wx/msw/regconf.h>
#endif

#include "cPanelCameraConfig.hpp"
#include "cPanelProgramConfig.hpp"

#include "cPreviewCameras.hpp"

#include "../recognize/src/types.hpp"
#include "../recognize/src/configuration_file.hpp"
#include "../recognize/src/recognize.hpp"

#include "types.hpp"

#include "widgets_helper.hpp"

enum MAIN_ids {
	BUTTON_TOGGLEPREVIEW = 1001,
	BTN_Recognize,
	CHK_RecognizeOnStart,
	BTN_ApplyChanges,
	BTN_UndoChanges,
	BTN_SaveToFile,
	BTN_AddCamera,
	BTN_RemoveCamera,
	TXT_FilePathInput,
	BTN_SearchFile
};

class cMain : public wxFrame {
	public:
		cMain(	wxLocale& locale, Recognize* recognize, 
				Configurations& configs, bool& recognizeActive, 
				wxConfig* appConfig, bool& mainClosed, 
				std::string filePath);
				
		~cMain();
		
		wxLocale& m_locale;
		
	protected:
		void OnQuitter(wxCloseEvent& event);
		
		void btnToggleRecognize_Clicked(wxCommandEvent& ev);
		void chkToggleRecognizeOnStart_Checked(wxCommandEvent& ev);
		void btnApplyChanges_Click(wxCommandEvent& ev);
		void btnUndoChanges_Click(wxCommandEvent& ev);
		void btnSaveToFile_Click(wxCommandEvent& ev);
		void btnAddCamera_Click(wxCommandEvent& ev);
		void btnRemoveCamera_Click(wxCommandEvent& ev);
		void btnSearchFile_Click(wxCommandEvent& ev);
		
	private:
		bool m_startRecognizeOnStart; // used to change the appConfig

		wxConfig* m_appConfig = nullptr;
		
		std::string m_filePath;

		wxPanel* m_root = nullptr;
		
		wxBookCtrlBase *m_book = nullptr;

		cPreviewCameras* m_preview = nullptr;
		
		wxButton* m_btnToggleRecogznie = nullptr;
		wxCheckBox* m_chkStartRecognizeOnStart = nullptr;

		SharedData m_sharedData;

		wxButton* m_btnApplyChanges;
		wxButton* m_btnUndoChanges;
		wxButton* m_btnSaveToFile;
		
		wxButton* m_btnAddCamera;
		wxButton* m_btnRemoveCamera;
		
		wxTextCtrl* m_txtFilePathInput;
		wxButton* m_btnSearchFile;
		
		Configurations m_tempConfig;
		
		bool* m_mainClosed;
		
		void AddProgramCamerasPages();

		wxDECLARE_EVENT_TABLE();
};