#include "cMain.hpp"
#include "cApp.hpp"

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
	EVT_BUTTON(MAIN_ids::BTN_Recognize, cMain::btnToggleRecognize_Clicked)
	EVT_CHECKBOX(MAIN_ids::CHK_RecognizeOnStart, cMain::chkToggleRecognizeOnStart_Checked)	
	EVT_BUTTON(MAIN_ids::BTN_ApplyChanges, cMain::btnApplyChanges_Click)
	EVT_BUTTON(MAIN_ids::BTN_UndoChanges, cMain::btnUndoChanges_Click)
	EVT_BUTTON(MAIN_ids::BTN_SaveToFile, cMain::btnSaveToFile_Click)
	EVT_BUTTON(MAIN_ids::BTN_AddCamera, cMain::btnAddCamera_Click)
	EVT_BUTTON(MAIN_ids::BTN_RemoveCamera, cMain::btnRemoveCamera_Click)
	EVT_BUTTON(MAIN_ids::BTN_SearchFile, cMain::btnSearchFile_Click)
wxEND_EVENT_TABLE()

cMain::cMain(	wxLocale& locale,
				Recognize* recognize, 
				Configurations& configs, 
				bool& recognizeActive, 
				wxConfig* appConfig, 
				bool& mainClosed,
				std::string filePath)
		: wxFrame(nullptr, wxID_ANY, _("Recognize"), wxPoint(30, 30), wxSize(1000, 700)), 
		m_appConfig(appConfig), m_mainClosed(&mainClosed), m_filePath(filePath), m_locale(locale) {
	/**
	 * m_root
	 *    |-- netbook
	 * 			|-- cameraPanel
	**/
		
	this->SetMinSize(wxSize(480, 640));

	this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(cMain::OnQuitter));

	this->m_sharedData.recognize = recognize;
	this->m_sharedData.configurations = &configs;
	this->m_sharedData.recognizeActive = &recognizeActive;
	this->m_startRecognizeOnStart = recognizeActive;
	
	this->m_tempConfig.camerasConfigs = configs.camerasConfigs;
	this->m_tempConfig.programConfig = configs.programConfig;

	// root panel
	this->m_root = new wxPanel(this, wxID_ANY);

	// sizer of root panel
	wxBoxSizer* hbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerCheck = new wxBoxSizer(wxVERTICAL);
	
	// create notebook
	m_book = new wxNotebook(this->m_root, wxID_ANY);

	this->AddProgramCamerasPages();

	this->m_chkStartRecognizeOnStart = new wxCheckBox(m_root, MAIN_ids::CHK_RecognizeOnStart, _("Start recognize on Start"));
	this->m_chkStartRecognizeOnStart->SetValue(this->m_startRecognizeOnStart);

	this->m_btnToggleRecogznie = new wxButton(m_root, MAIN_ids::BTN_Recognize, recognizeActive ? _("Stop Recognize") : _("Start Recognize"));
	this->m_btnToggleRecogznie->SetBackgroundColour(wxColor(200, 50, 50));
	
	this->m_btnApplyChanges = new wxButton(this->m_root, MAIN_ids::BTN_ApplyChanges, _("Apply Changes"), wxDefaultPosition, wxDefaultSize);
	this->m_btnApplyChanges->Enable(false);
	this->m_btnUndoChanges = new wxButton(this->m_root, MAIN_ids::BTN_UndoChanges, _("Undo Changes"), wxDefaultPosition, wxDefaultSize);
	this->m_btnSaveToFile = new wxButton(this->m_root, MAIN_ids::BTN_SaveToFile, _("Save to file"), wxDefaultPosition, wxDefaultSize);

	this->m_sharedData.btnApplyChanges = this->m_btnApplyChanges;

	sizerCheck->Add(this->m_chkStartRecognizeOnStart, 0, wxGROW, 0);
	sizerCheck->AddStretchSpacer();
	sizerCheck->Add(this->m_btnToggleRecogznie, 0, wxGROW, 0);

	this->m_btnAddCamera = new wxButton(this->m_root, MAIN_ids::BTN_AddCamera, _("Add camera"), wxDefaultPosition, wxDefaultSize);
	this->m_btnRemoveCamera = new wxButton(this->m_root, MAIN_ids::BTN_RemoveCamera, _("Remove selected camera"), wxDefaultPosition, wxDefaultSize);
		
	this->m_txtFilePathInput = new wxTextCtrl(this->m_root, MAIN_ids::TXT_FilePathInput, m_filePath);
	this->m_btnSearchFile = new wxButton(this->m_root, MAIN_ids::BTN_SearchFile, _("Search file"));

	sizerButtons->Add(m_btnApplyChanges, 1, wxGROW | wxALL, 5);
	sizerButtons->Add(m_btnUndoChanges, 1, wxGROW | wxALL, 5);
	sizerButtons->Add(m_btnSaveToFile, 1, wxGROW | wxALL, 5);
		
	sizerTop->Add(sizerCheck, 1, wxTOP | wxLEFT | wxRIGHT | wxGROW, 10);
	
	wxSizer* sizerInputFile = new wxBoxSizer(wxVERTICAL);
	sizerInputFile->Add(WidgetsHelper::GetSizerItemLabel(this->m_root, this->m_txtFilePathInput, _("File")), 2, wxGROW);
	sizerInputFile->Add(this->m_btnSearchFile, 1, wxGROW);
	sizerTop->Add(sizerInputFile, 3, wxTOP | wxRIGHT, 10);
				  
	sizerTop->Add(WidgetsHelper::JoinWidgetsOnSizerV(this->m_btnAddCamera, this->m_btnRemoveCamera, 5), 1, wxTOP, 30);
		
	hbox->Add(sizerTop, 1, wxGROW | wxBOTTOM, 5);

	// add netbook to sizer
	hbox->Add(this->m_book, 1, wxGROW | wxALIGN_TOP | wxBOTTOM, 5);
	
	hbox->Add(sizerButtons, 1, wxGROW);

	// set sizer of panel
	this->m_root->SetSizer(hbox);

	// center the panel
	this->Centre();
};

cMain::~cMain() { };


void cMain::AddProgramCamerasPages() {	
	// create camera panel
	cPanelProgramConfig* programPanel = new cPanelProgramConfig(this->m_book, this->m_tempConfig.programConfig, &this->m_sharedData);

	// add program config to netbook
	m_book->AddPage(programPanel, _("Program"), false);

	for (size_t i = 0; i < this->m_tempConfig.camerasConfigs.size(); i++) {	
		// create camera panel		
		cPanelCameraConfig* cameraPanel = new cPanelCameraConfig(this->m_book, &this->m_tempConfig.camerasConfigs[i], &this->m_sharedData);

		// add camera panel to netbook
		m_book->AddPage(cameraPanel, this->m_tempConfig.camerasConfigs[i].cameraName, false);
	}
}

void cMain::btnToggleRecognize_Clicked(wxCommandEvent& ev) {
	if (*this->m_sharedData.recognizeActive) {
		*this->m_sharedData.recognizeActive = false;
		this->m_sharedData.recognize->CloseAndJoin();
		this->m_btnToggleRecogznie->SetLabel(_("Start Recognize"));
	} else {
		this->m_sharedData.recognize->Start(std::ref(*this->m_sharedData.configurations), false, this->m_sharedData.configurations->programConfig.telegramConfig.useTelegramBot);
		*this->m_sharedData.recognizeActive = true;
		this->m_btnToggleRecogznie->SetLabel(_("Stop Recognize"));
	}
}

void cMain::chkToggleRecognizeOnStart_Checked(wxCommandEvent& ev) {
	this->m_startRecognizeOnStart = !this->m_startRecognizeOnStart;
	this->m_appConfig->Write("StartRecognizeOnStart", this->m_startRecognizeOnStart);
}

void cMain::OnQuitter(wxCloseEvent& event) {
	
	if (!*this->m_mainClosed) {
		*this->m_mainClosed = true;

		if (*this->m_sharedData.recognizeActive) {		
			*this->m_sharedData.recognizeActive = false;
			this->m_sharedData.recognize->CloseAndJoin();
		}
				
		try {
			this->Close();
		} catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
		}

		std::cout << "Closed." << std::endl;
		
		event.Skip();
		event.StopPropagation();
	}
}

void cMain::btnApplyChanges_Click(wxCommandEvent& ev) {
	if (*this->m_sharedData.recognizeActive) {
		// Stop recognize
		this->m_sharedData.recognize->CloseAndJoin();
		*this->m_sharedData.recognizeActive = false;
		
		// copy temp configs
		this->m_sharedData.configurations->programConfig = this->m_tempConfig.programConfig;
		this->m_sharedData.configurations->camerasConfigs = this->m_tempConfig.camerasConfigs;
		
		// Start recognize
		this->m_sharedData.recognize->Start(*this->m_sharedData.configurations, false,this->m_sharedData.configurations->programConfig.telegramConfig.useTelegramBot);
		
		*this->m_sharedData.recognizeActive = true;
	} else {
		// copy temp configs
		this->m_sharedData.configurations->programConfig = this->m_tempConfig.programConfig;
		this->m_sharedData.configurations->camerasConfigs = this->m_tempConfig.camerasConfigs;
	}
		
	this->m_btnApplyChanges->Enable(false);
	
	ev.Skip();
}

void cMain::btnUndoChanges_Click(wxCommandEvent& ev) {
	size_t lastSelection = this->m_book->GetSelection();
	
	// restore configs to original
	this->m_tempConfig.programConfig = this->m_sharedData.configurations->programConfig;
	this->m_tempConfig.camerasConfigs = this->m_sharedData.configurations->camerasConfigs;
	
	this->m_book->DeleteAllPages();
	
	this->AddProgramCamerasPages();
	
	if (lastSelection < this->m_book->GetPageCount())
		this->m_book->ChangeSelection(lastSelection);
	
	this->m_btnApplyChanges->Enable(false);
	
	ev.Skip();
}

void cMain::btnSaveToFile_Click(wxCommandEvent& ev) {
	std::string p = this->m_txtFilePathInput->GetValue().ToStdString();
	std::cout << "Writing to: " << p << std::endl;
	ConfigurationFile::SaveConfigurations(*this->m_sharedData.configurations, p);
	ev.Skip();
}

void cMain::btnAddCamera_Click(wxCommandEvent& ev) {
	this->m_tempConfig.camerasConfigs.push_back(CameraConfiguration());
	
	this->m_tempConfig.camerasConfigs.back().cameraName = _("new camera");
	
	cPanelCameraConfig* cameraPanel = new cPanelCameraConfig(this->m_book, &this->m_tempConfig.camerasConfigs.back(), &this->m_sharedData);

	// add camera panel to netbook
	m_book->AddPage(cameraPanel, this->m_tempConfig.camerasConfigs.back().cameraName, false);
}

void cMain::btnRemoveCamera_Click(wxCommandEvent& ev) {
	int s = this->m_book->GetSelection();
	
	if (s == 0) {
		wxMessageBox(_("Cannot delete the program configuration."), _("Couldn't delete the camera"));
	} else {
		this->m_book->RemovePage(s);
		this->m_sharedData.configurations->camerasConfigs.erase(this->m_sharedData.configurations->camerasConfigs.begin() + s - 1);
		this->m_tempConfig.camerasConfigs.erase(this->m_tempConfig.camerasConfigs.begin() + s - 1);
	}
}

void cMain::btnSearchFile_Click(wxCommandEvent& ev) {	
	bool proceed = true;
	if (this->m_btnApplyChanges->IsEnabled()) {
		wxMessageDialog dialog(this,
							   _("There are unsaved changes, do you want to apply the changes and save them to the file?"),
							   _("Warning"),
							   wxCENTER |
							   wxNO_DEFAULT | wxYES_NO | wxCANCEL |
							   wxICON_EXCLAMATION);

		wxCommandEvent* ev = new wxCommandEvent(wxEVT_BUTTON, wxID_ANY);
		switch (dialog.ShowModal()) {
			case wxID_YES:
				this->btnApplyChanges_Click(*ev);
				this->btnSaveToFile_Click(*ev);
				break;

//			case wxID_NO:
//				break;

			case wxID_CANCEL:
				proceed = false;
				break;
		}
	}
	
	if (proceed) {
		static wxString s_extDef;
		wxString path = wxFileSelector(
							_("Select the file to load"),
							wxEmptyString, wxEmptyString,
							s_extDef,
							wxString::Format (
								_("Configuration file (*.ini)|*.ini|Plain text (*.txt)|*.txt|All files (%s)|%s"),
								wxFileSelectorDefaultWildcardStr,
								wxFileSelectorDefaultWildcardStr
							),
							wxFD_OPEN|wxFD_CHANGE_DIR|wxFD_PREVIEW,
							this
					   );
		
		if (!path.empty()) {
			this->m_book->DeleteAllPages();
			
			this->m_tempConfig = ConfigurationFile::ReadConfigurations(path.ToStdString());
	
			*this->m_sharedData.configurations = this->m_tempConfig;
	
			this->AddProgramCamerasPages();
			
			this->m_appConfig->Write("LastConfigFile", path);
			this->m_txtFilePathInput->SetValue(path);
		}
	}
}