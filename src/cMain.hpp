#pragma once

#include <wx/bookctrl.h>
#include <wx/frame.h>

#include "cPanelCameraConfig.hpp"
#include "cPanelProgramConfig.hpp"

#include "cPreviewCameras.hpp"

#include "../recognize/src/types.hpp"
#include "../recognize/src/configuration_file.hpp"
#include "../recognize/src/recognize.hpp"

#include "types.hpp"

#include <wx/thread.h>
#include <wx/event.h>

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#ifdef _WIN32 
#include <wx/msw/regconf.h>
#endif

enum MAIN_ids {
	BUTTON_TOGGLEPREVIEW = 1001,
	CHK_Recognize,
	CHK_RecognizeOnStart,
	BTN_ApplyChanges,
	BTN_UndoChanges,
	BTN_SaveToFile,
	BTN_AddCamera,
	BTN_RemoveCamera
};

class cMain : public wxFrame {
	public:
		cMain(Recognize* recognize, Configuration* configFile, bool& recognizeActive, wxConfig* appConfig, bool& mainClosed);
		~cMain();

	protected:
		void chkToggleRecognize_Checked(wxCommandEvent& ev);
		void chkToggleRecognizeOnStart_Checked(wxCommandEvent& ev);
		void btnApplyChanges_Click(wxCommandEvent& ev);
		void btnUndoChanges_Click(wxCommandEvent& ev);
		void btnSaveToFile_Click(wxCommandEvent& ev);
		void btnAddCamera_Click(wxCommandEvent& ev);
		void btnRemoveCamera_Click(wxCommandEvent& ev);
		void OnQuitter(wxCloseEvent& event);
		
	private:
		bool m_startRecognizeOnStart; // used to change the appConfig

		wxConfig* m_appConfig = nullptr;
		
		Configuration* m_configFile;

		wxPanel* m_root = nullptr;
		
		wxBookCtrlBase *m_book = nullptr;

		cPreviewCameras* m_preview = nullptr;
		
		wxCheckBox* m_chkRecognizeActive = nullptr;
		wxCheckBox* m_chkStartRecognizeOnStart = nullptr;		

		SharedData m_sharedData;

		wxButton* m_btnApplyChanges;
		wxButton* m_btnUndoChanges;
		wxButton* m_btnSaveToFile;
		
		wxButton* m_btnAddCamera;
		wxButton* m_btnRemoveCamera;
		
		Configurations m_tempConfig;
		
		bool* m_mainClosed;
		
		void AddProgramCamerasPages();

		wxDECLARE_EVENT_TABLE();
};