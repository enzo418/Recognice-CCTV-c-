#include "cPanelProgramConfig.hpp"

wxBEGIN_EVENT_TABLE(cPanelProgramConfig, wxPanel)
	EVT_SPINCTRL(PROGRAM_ids::SPIN_MsBetweenFrames, cPanelProgramConfig::spinMsBetweenFrames_SpinChange)
	EVT_SPINCTRLDOUBLE(PROGRAM_ids::SPIN_RatioScaleOutput, cPanelProgramConfig::spinRatioScaleOutput_SpinChange)
	EVT_CHECKBOX(PROGRAM_ids::CHK_ShowPreviewCameras, cPanelProgramConfig::chkShowPreviewCameras_CheckBoxClick)
	EVT_CHECKBOX(PROGRAM_ids::CHK_ShowAreaCameraSees, cPanelProgramConfig::chkShowAreaCameraSees_CheckBoxClick)
	EVT_CHECKBOX(PROGRAM_ids::CHK_ShowProcessedImages, cPanelProgramConfig::chkShowProcessedImages_CheckBoxClick)
	EVT_CHECKBOX(PROGRAM_ids::CHK_UseTelegramBot, cPanelProgramConfig::chkUseTelegramBot_CheckBoxClick)
	EVT_SPINCTRL(PROGRAM_ids::SPIN_SecondsBetweenImage, cPanelProgramConfig::spinSecondsBetweenImage_SpinChange)
	EVT_SPINCTRL(PROGRAM_ids::SPIN_SecondsBetweenMessage, cPanelProgramConfig::spinSecondsBetweenMessage_SpinChange)
	EVT_CHECKBOX(PROGRAM_ids::CHK_SendImageAfterDetectigChange, cPanelProgramConfig::chkSendImageAfterDetectigChange_CheckBoxClick)	
	EVT_CHECKBOX(PROGRAM_ids::CHK_SendImageOfAllCameras, cPanelProgramConfig::chkSendImageOfAllCameras_CheckBoxClick)
	EVT_CHECKBOX(PROGRAM_ids::CHK_UseGifInsteadOfImage, cPanelProgramConfig::chkUseGifInsteadOfImage_CheckBoxClick)	
	EVT_COMBOBOX(PROGRAM_ids::COMBO_GifQuality, cPanelProgramConfig::comboGifQuality_Select)
	EVT_SPINCTRL(PROGRAM_ids::SPIN_FramesBefore, cPanelProgramConfig::spinFramesBefore_SpinChange)
	EVT_SPINCTRL(PROGRAM_ids::SPIN_FramesAfter, cPanelProgramConfig::spinFramesAfter_SpinChange)
wxEND_EVENT_TABLE()

cPanelProgramConfig::cPanelProgramConfig(wxBookCtrlBase* parent, ProgramConfiguration& progConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(&progConfig), m_sharedData(sharedData) {
	
	wxSizer* sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Tweak configuration");
	wxSizer* sizerLeft = new wxStaticBoxSizer(wxVERTICAL, this, "");
	wxSizer* sizerRight = new wxStaticBoxSizer(wxVERTICAL, this, "");

	int imax = std::numeric_limits<int>::max();
	double dmax = std::numeric_limits<double>::max();

	wxStaticText* labelMsBetweenFrames = new wxStaticText(this, wxID_ANY, "ms Between Frame", wxDefaultPosition, wxDefaultSize);
	this->m_spinMsBetweenFrames = new wxSpinCtrl(this, PROGRAM_ids::SPIN_MsBetweenFrames, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->msBetweenFrame);;

	wxStaticText* labelRatioScaleOutput = new wxStaticText(this, wxID_ANY, "Scale of preview", wxDefaultPosition, wxDefaultSize);
	this->m_spinRatioScaleOutput = new wxSpinCtrlDouble(this, PROGRAM_ids::SPIN_RatioScaleOutput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->ratioScaleOutput);
	this->m_spinRatioScaleOutput->SetDigits(2);

	this->m_chkShowPreviewCameras = new wxCheckBox(this, PROGRAM_ids::CHK_ShowPreviewCameras, wxT("Show preview of the cameras"));
	this->m_chkShowPreviewCameras->SetValue(m_config->showPreview);

	this->m_chkShowAreaCameraSees = new wxCheckBox(this, PROGRAM_ids::CHK_ShowAreaCameraSees, wxT("Show only the camera ROI"));
	this->m_chkShowAreaCameraSees->SetValue(m_config->showAreaCameraSees);

	this->m_chkShowProcessedImages = new wxCheckBox(this, PROGRAM_ids::CHK_ShowProcessedImages, wxT("Show processed images"));
	this->m_chkShowProcessedImages->SetValue(m_config->showProcessedFrames);

	// -- Telegram Bot config
	this->m_chkUseTelegramBot = new wxCheckBox(this, PROGRAM_ids::CHK_UseTelegramBot, wxT("Use telegram bot"));
	this->m_chkUseTelegramBot->SetValue(m_config->telegramConfig.useTelegramBot);

	wxStaticText* labelTelegramBotApiKey = new wxStaticText(this, wxID_ANY, "Telegram API key", wxDefaultPosition, wxDefaultSize);
	this->m_txtTelegramBotApiKey = new wxTextCtrl(this, PROGRAM_ids::TXT_TelegramBotApiKey, m_config->telegramConfig.apiKey, wxDefaultPosition, wxDefaultSize);
	this->m_txtTelegramBotApiKey->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtTelegramBotApiKey_KillFocus, this);

	wxStaticText* labelTelegramChatId = new wxStaticText(this, wxID_ANY, "Telegram CHAT id", wxDefaultPosition, wxDefaultSize);
	this->m_txtTelegramChatId = new wxTextCtrl(this, PROGRAM_ids::TXT_TelegramChatId, m_config->telegramConfig.chatId, wxDefaultPosition, wxDefaultSize);
	this->m_txtTelegramChatId->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtTelegramChatId_KillFocus, this);

	wxStaticText* labelAuthUsersToSendActions = new wxStaticText(this, wxID_ANY, "Authorized users to send actions", wxDefaultPosition, wxDefaultSize);
	this->m_txtAuthUsersToSendActions = new wxTextCtrl(this, PROGRAM_ids::TXT_AuthUsersToSendActions, Utils::VectorToCommaString(this->m_config->authUsersToSendActions), wxDefaultPosition, wxDefaultSize);
	this->m_txtAuthUsersToSendActions->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtAuthUsersToSendActions_KillFocus, this);
	
	wxStaticText* labelSecondsBetweenImage = new wxStaticText(this, wxID_ANY, "Seconds between image message", wxDefaultPosition, wxDefaultSize);
	this->m_spinSecondsBetweenImage = new wxSpinCtrl(this, PROGRAM_ids::SPIN_SecondsBetweenImage, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->secondsBetweenImage);
	
	wxStaticText* labelSecondsBetweenMessage = new wxStaticText(this, wxID_ANY, "Seconds between text message", wxDefaultPosition, wxDefaultSize);
	this->m_spinSecondsBetweenMessage = new wxSpinCtrl(this, PROGRAM_ids::SPIN_SecondsBetweenMessage, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->secondsBetweenMessage);
	
	this->m_chkSendImageAfterDetectigChange = new wxCheckBox(this, PROGRAM_ids::CHK_SendImageAfterDetectigChange, wxT("Send image after detectig change"));
	this->m_chkSendImageAfterDetectigChange->SetValue(m_config->sendImageWhenDetectChange);

	this->m_chkSendImageOfAllCameras = new wxCheckBox(this, PROGRAM_ids::CHK_SendImageOfAllCameras, wxT("Send image of all cameras"));
	this->m_chkSendImageOfAllCameras->SetValue(m_config->sendImageOfAllCameras);
	
	this->m_chkUseGifInsteadOfImage = new wxCheckBox(this, PROGRAM_ids::CHK_UseGifInsteadOfImage, wxT("Use gif instead of image"));
	this->m_chkUseGifInsteadOfImage->SetValue(m_config->useGifInsteadImage);

	wxStaticText* labelGifQuality = new wxStaticText(this, wxID_ANY, "Gif resize level (higher = lower quality)", wxDefaultPosition, wxDefaultSize);
	this->m_comboGifQuality = new wxComboBox(this, PROGRAM_ids::COMBO_GifQuality, wxString("High"), wxDefaultPosition, wxDefaultSize);
	m_comboGifQuality->Append(wxString("Very High"));
	m_comboGifQuality->Append(wxString("High"));
	m_comboGifQuality->Append(wxString("Medium"));
	m_comboGifQuality->Append(wxString("Low"));
	m_comboGifQuality->Append(wxString("None"));
	
	m_comboGifQuality->SetValue(Utils::GifResizePercentageToString(this->m_config->gifResizePercentage));

	// -- Widgets GIF FRAMES

	wxBoxSizer* sizerGifFrames = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* labelFramesBefore = new wxStaticText(this, wxID_ANY, "Frames Before", wxDefaultPosition, wxDefaultSize);
	wxStaticText* labelFramesAfter = new wxStaticText(this, wxID_ANY, "Frames After", wxDefaultPosition, wxDefaultSize);
//	wxStaticText* labelFrameDetection = new wxStaticText(this, wxID_ANY, "..", wxDefaultPosition, wxDefaultSize);
	this->m_spinFramesBefore = new wxSpinCtrl(this, PROGRAM_ids::SPIN_FramesBefore, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->numberGifFrames.framesBefore);
	this->m_spinFramesAfter = new wxSpinCtrl(this, PROGRAM_ids::SPIN_FramesAfter, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->numberGifFrames.framesAfter);
	
	wxBoxSizer* sizerFramesBefore = new wxBoxSizer(wxVERTICAL);
	sizerFramesBefore->Add(labelFramesBefore, 0, wxGROW);
	sizerFramesBefore->Add(m_spinFramesBefore, 0, wxGROW);
	
	wxBoxSizer* sizerFramesAfter = new wxBoxSizer(wxVERTICAL);
	sizerFramesAfter->Add(labelFramesAfter, 0, wxGROW);
	sizerFramesAfter->Add(m_spinFramesAfter, 0, wxGROW);
	
	sizerGifFrames->Add(sizerFramesBefore, 1, wxGROW);
//	sizerGifFrames->Add(labelFrameDetection, 0, wxGROW);
	sizerGifFrames->Add(sizerFramesAfter, 1, wxGROW);
	

	// -- Add widgets to sizer
	sizerLeft->Add(labelMsBetweenFrames, 0, wxALL | wxGROW, 0);		
	sizerLeft->Add(m_spinMsBetweenFrames, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerLeft->Add(labelRatioScaleOutput, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(m_spinRatioScaleOutput, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);
	
	sizerLeft->Add(m_chkShowPreviewCameras, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerLeft->Add(m_chkShowAreaCameraSees, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);
	
	sizerLeft->Add(m_chkShowProcessedImages, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerLeft->Add(m_chkUseTelegramBot, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerLeft->Add(m_chkSendImageAfterDetectigChange, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerLeft->Add(m_chkSendImageOfAllCameras, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerLeft->Add(m_chkUseGifInsteadOfImage, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerRight->Add(labelTelegramBotApiKey, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_txtTelegramBotApiKey, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerRight->Add(labelTelegramChatId, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_txtTelegramChatId, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerRight->Add(labelAuthUsersToSendActions, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_txtAuthUsersToSendActions, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerRight->Add(labelSecondsBetweenImage, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinSecondsBetweenImage, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerRight->Add(labelSecondsBetweenMessage, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinSecondsBetweenMessage, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);

	sizerRight->Add(labelGifQuality, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_comboGifQuality, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);

//	sizerRight->Add(labelMaxGifFrames, 0, wxALL | wxGROW, 0);
	sizerRight->Add(sizerGifFrames, 0, wxALL | wxGROW, 0);
//	sizerRight->Add(5, 5, 0, wxGROW | wxALL, 5);
	
	sizer->Add(sizerLeft, 2, wxGROW, 0);
	sizer->Add(sizerRight, 2, wxGROW, 0);

	this->SetSizer(sizer);
}

cPanelProgramConfig::~cPanelProgramConfig() {

}

void cPanelProgramConfig::spinMsBetweenFrames_SpinChange(wxSpinEvent& ev) {
	std::cout << "ms: " << this->m_spinMsBetweenFrames->GetValue() << std::endl;
	 this->m_config->msBetweenFrame = this->m_spinMsBetweenFrames->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::spinRatioScaleOutput_SpinChange(wxSpinDoubleEvent& ev) {
	this->m_config->ratioScaleOutput = this->m_spinRatioScaleOutput->GetValue();
	std::cout << "ratio: " <<  this->m_spinRatioScaleOutput->GetValue() << std::endl;
	ev.Skip();
}

void cPanelProgramConfig::chkShowPreviewCameras_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->showPreview = !this->m_config->showPreview;
	ev.Skip();
}

void cPanelProgramConfig::chkShowAreaCameraSees_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->showAreaCameraSees = !this->m_config->showAreaCameraSees;
	ev.Skip();
}

void cPanelProgramConfig::chkShowProcessedImages_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->showProcessedFrames = !this->m_config->showProcessedFrames;
	ev.Skip();
}

void cPanelProgramConfig::chkUseTelegramBot_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->telegramConfig.useTelegramBot = !this->m_config->telegramConfig.useTelegramBot;
	ev.Skip();
}

void cPanelProgramConfig::txtTelegramBotApiKey_KillFocus(wxFocusEvent& ev) {
	this->m_config->telegramConfig.apiKey = this->m_txtTelegramBotApiKey->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::txtTelegramChatId_KillFocus(wxFocusEvent& ev) {
	this->m_config->telegramConfig.chatId = this->m_txtTelegramChatId->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::txtAuthUsersToSendActions_KillFocus(wxFocusEvent& ev) {
	this->m_config->authUsersToSendActions = Utils::SplitString(std::string(this->m_txtAuthUsersToSendActions->GetValue()), ",");
	ev.Skip();
}

void cPanelProgramConfig::spinSecondsBetweenImage_SpinChange(wxSpinEvent& ev) {
	this->m_config->secondsBetweenImage = this->m_spinSecondsBetweenImage->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::spinSecondsBetweenMessage_SpinChange(wxSpinEvent& ev) {
	this->m_config->secondsBetweenMessage = this->m_spinSecondsBetweenMessage->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::chkSendImageAfterDetectigChange_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->sendImageWhenDetectChange = !this->m_config->sendImageWhenDetectChange;
	ev.Skip();
}

void cPanelProgramConfig::chkSendImageOfAllCameras_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->sendImageOfAllCameras = !this->m_config->sendImageOfAllCameras;
	ev.Skip();
}

void cPanelProgramConfig::chkUseGifInsteadOfImage_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->useGifInsteadImage = !this->m_config->sendImageOfAllCameras;
	ev.Skip();
}

void cPanelProgramConfig::comboGifQuality_Select(wxCommandEvent& ev) {
	wxString val = ev.GetString();
	this->m_config->gifResizePercentage = Utils::GifResizePercentageFromString(val.ToStdString());
	std::cout << "gif: " << this->m_config->gifResizePercentage << std::endl;
	ev.Skip();
}

void cPanelProgramConfig::spinFramesBefore_SpinChange(wxSpinEvent& ev) {
	this->m_config->numberGifFrames.framesBefore = ev.GetValue();
	ev.Skip();
}

void cPanelProgramConfig::spinFramesAfter_SpinChange(wxSpinEvent& ev) {
	this->m_config->numberGifFrames.framesAfter = ev.GetValue();
	ev.Skip();
}