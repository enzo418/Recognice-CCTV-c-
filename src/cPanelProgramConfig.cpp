#include "cPanelProgramConfig.hpp"

#define BIND(Widget_, EventTag_, EventHandler_, Function_) Widget_->Bind(EventTag_, \
																			[this](EventHandler_& ev) { \
																				this->m_sharedData->btnApplyChanges->Enable(true); \
																				Function_(ev); \
																			})

cPanelProgramConfig::cPanelProgramConfig(wxBookCtrlBase* parent, ProgramConfiguration& progConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(&progConfig), m_sharedData(sharedData) {
	
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerLeft = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);

	int imax = std::numeric_limits<int>::max();
	double dmax = std::numeric_limits<double>::max();

//	wxStaticText* labelMsBetweenFrames = new wxStaticText(this, wxID_ANY, "ms Between Frame", wxDefaultPosition, wxDefaultSize);
	this->m_spinMsBetweenFrames = new wxSpinCtrl(this, PROGRAM_ids::SPIN_MsBetweenFrames, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->msBetweenFrame);;
	BIND(m_spinMsBetweenFrames, wxEVT_SPINCTRL, wxSpinEvent, spinMsBetweenFrames_SpinChange);
	
	this->m_spinRatioScaleOutput = new wxSpinCtrlDouble(this, PROGRAM_ids::SPIN_RatioScaleOutput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->ratioScaleOutput);
	this->m_spinRatioScaleOutput->SetDigits(2);
	BIND(m_spinRatioScaleOutput, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinRatioScaleOutput_SpinChange);

	this->m_chkShowPreviewCameras = new wxCheckBox(this, PROGRAM_ids::CHK_ShowPreviewCameras, wxT("Show preview of the cameras"));
	this->m_chkShowPreviewCameras->SetValue(m_config->showPreview);
	BIND(m_chkShowPreviewCameras, wxEVT_CHECKBOX, wxCommandEvent, chkShowPreviewCameras_CheckBoxClick);

	this->m_chkShowAreaCameraSees = new wxCheckBox(this, PROGRAM_ids::CHK_ShowAreaCameraSees, wxT("Show only the camera ROI"));
	this->m_chkShowAreaCameraSees->SetValue(m_config->showAreaCameraSees);
	BIND(m_chkShowAreaCameraSees, wxEVT_CHECKBOX, wxCommandEvent, chkShowAreaCameraSees_CheckBoxClick);
	
	this->m_chkShowProcessedImages = new wxCheckBox(this, PROGRAM_ids::CHK_ShowProcessedImages, wxT("Show processed images"));
	this->m_chkShowProcessedImages->SetValue(m_config->showProcessedFrames);
	BIND(m_chkShowProcessedImages, wxEVT_CHECKBOX, wxCommandEvent, chkShowProcessedImages_CheckBoxClick);

	// -- Telegram Bot config
	this->m_chkUseTelegramBot = new wxCheckBox(this, PROGRAM_ids::CHK_UseTelegramBot, wxT("Use telegram bot"));
	this->m_chkUseTelegramBot->SetValue(m_config->telegramConfig.useTelegramBot);
	BIND(m_chkUseTelegramBot, wxEVT_CHECKBOX, wxCommandEvent, chkUseTelegramBot_CheckBoxClick);

	this->m_txtTelegramBotApiKey = new wxTextCtrl(this, PROGRAM_ids::TXT_TelegramBotApiKey, m_config->telegramConfig.apiKey, wxDefaultPosition, wxDefaultSize);
	this->m_txtTelegramBotApiKey->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtTelegramBotApiKey_KillFocus, this);
	BIND(m_txtTelegramBotApiKey, wxEVT_KILL_FOCUS, wxFocusEvent, txtTelegramBotApiKey_KillFocus);

	this->m_txtTelegramChatId = new wxTextCtrl(this, PROGRAM_ids::TXT_TelegramChatId, m_config->telegramConfig.chatId, wxDefaultPosition, wxDefaultSize);
	BIND(m_txtTelegramChatId, wxEVT_KILL_FOCUS, wxFocusEvent, txtTelegramChatId_KillFocus);
	
	this->m_txtAuthUsersToSendActions = new wxTextCtrl(this, PROGRAM_ids::TXT_AuthUsersToSendActions, Utils::VectorToCommaString(this->m_config->authUsersToSendActions), wxDefaultPosition, wxDefaultSize);
	this->m_txtAuthUsersToSendActions->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtAuthUsersToSendActions_KillFocus, this);
	BIND(m_txtAuthUsersToSendActions, wxEVT_KILL_FOCUS, wxFocusEvent, txtAuthUsersToSendActions_KillFocus);
	
	this->m_spinSecondsBetweenImage = new wxSpinCtrl(this, PROGRAM_ids::SPIN_SecondsBetweenImage, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->secondsBetweenImage);
	BIND(m_spinSecondsBetweenImage, wxEVT_SPINCTRL, wxSpinEvent, spinSecondsBetweenImage_SpinChange);
	
	this->m_spinSecondsBetweenMessage = new wxSpinCtrl(this, PROGRAM_ids::SPIN_SecondsBetweenMessage, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->secondsBetweenMessage);
	BIND(m_spinSecondsBetweenMessage, wxEVT_SPINCTRL, wxSpinEvent, spinSecondsBetweenMessage_SpinChange);
	
	this->m_chkSendImageAfterDetectigChange = new wxCheckBox(this, PROGRAM_ids::CHK_SendImageAfterDetectigChange, wxT("Send image after detectig change"));
	this->m_chkSendImageAfterDetectigChange->SetValue(m_config->sendImageWhenDetectChange);
	BIND(m_chkSendImageAfterDetectigChange, wxEVT_CHECKBOX, wxCommandEvent, chkSendImageAfterDetectigChange_CheckBoxClick);

	this->m_chkSendImageOfAllCameras = new wxCheckBox(this, PROGRAM_ids::CHK_SendImageOfAllCameras, wxT("Send image of all cameras"));
	this->m_chkSendImageOfAllCameras->SetValue(m_config->sendImageOfAllCameras);
	BIND(m_chkSendImageOfAllCameras, wxEVT_CHECKBOX, wxCommandEvent, chkSendImageOfAllCameras_CheckBoxClick);
	
	this->m_chkUseGifInsteadOfImage = new wxCheckBox(this, PROGRAM_ids::CHK_UseGifInsteadOfImage, wxT("Use gif instead of image"));
	this->m_chkUseGifInsteadOfImage->SetValue(m_config->useGifInsteadImage);
	BIND(m_chkUseGifInsteadOfImage, wxEVT_CHECKBOX, wxCommandEvent, chkUseGifInsteadOfImage_CheckBoxClick);

	this->m_comboGifQuality = new wxComboBox(this, PROGRAM_ids::COMBO_GifQuality, wxString("High"), wxDefaultPosition, wxDefaultSize);
	m_comboGifQuality->Append(wxString("Very High"));
	m_comboGifQuality->Append(wxString("High"));
	m_comboGifQuality->Append(wxString("Medium"));
	m_comboGifQuality->Append(wxString("Low"));
	m_comboGifQuality->Append(wxString("None"));
	m_comboGifQuality->SetValue(Utils::GifResizePercentageToString(this->m_config->gifResizePercentage));
	BIND(m_comboGifQuality, wxEVT_COMBOBOX, wxCommandEvent, comboGifQuality_Select);
		
	this->m_spinFramesBefore = new wxSpinCtrl(this, PROGRAM_ids::SPIN_FramesBefore, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->numberGifFrames.framesBefore);
	BIND(m_spinFramesBefore, wxEVT_SPINCTRL, wxSpinEvent, spinFramesBefore_SpinChange);
	
	this->m_spinFramesAfter = new wxSpinCtrl(this, PROGRAM_ids::SPIN_FramesAfter, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->numberGifFrames.framesAfter);
	BIND(m_spinFramesAfter, wxEVT_SPINCTRL, wxSpinEvent, spinFramesAfter_SpinChange);
	
	this->m_chkShowIgnoredAreas = new wxCheckBox(this, wxID_ANY, wxT("Show ignored areas"));
	this->m_chkShowIgnoredAreas->SetValue(m_config->showIgnoredAreas);
	BIND(m_chkShowIgnoredAreas, wxEVT_CHECKBOX, wxCommandEvent, chkShowIgnoredAreas_CheckBoxClick);
	
	// --------------- Sizers
	const int flags = wxALL | wxGROW;
	const int border = 10;
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinMsBetweenFrames, wxT("Milliseconds between frame")), 
						WidgetsHelper::GetSizerItemLabel(this, m_spinRatioScaleOutput, wxT("Ratio Scale output")), 
						5
					), 0, flags, border);
	
	sizerLeft->Add(m_chkShowPreviewCameras, 0, flags, border);

	sizerLeft->Add(m_chkShowAreaCameraSees, 0, flags, border);
	
	sizerLeft->Add(m_chkShowProcessedImages, 0, flags, border);

	sizerLeft->Add(m_chkUseTelegramBot, 0, flags, border);

	sizerLeft->Add(m_chkSendImageAfterDetectigChange, 0, flags, border);

	sizerLeft->Add(m_chkSendImageOfAllCameras, 0, flags, border);

	sizerLeft->Add(m_chkUseGifInsteadOfImage, 0, flags, border);
	
	sizerLeft->Add(m_chkShowIgnoredAreas, 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtTelegramBotApiKey, wxT("Telegram bot api key")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtTelegramChatId, wxT("Telegram bot chat id")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtAuthUsersToSendActions, wxT("Users athorized to send actions")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinSecondsBetweenImage, wxT("Seconds between message with image")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinSecondsBetweenMessage, wxT("Seconds between message with text")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_comboGifQuality, wxT("Gif resize level (higher = lower quality)")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesBefore, wxT("Frames Before")),
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAfter, wxT("Frames After")),
						5
					), 0, flags, border);
	
	sizer->Add(sizerLeft, 2, wxGROW, 0);
	sizer->Add(sizerRight, 2, wxGROW, 0);

	this->SetSizer(sizer);
}

cPanelProgramConfig::~cPanelProgramConfig() { }

void cPanelProgramConfig::spinMsBetweenFrames_SpinChange(wxSpinEvent& ev) {
	this->m_config->msBetweenFrame = this->m_spinMsBetweenFrames->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::spinRatioScaleOutput_SpinChange(wxSpinDoubleEvent& ev) {
	this->m_config->ratioScaleOutput = this->m_spinRatioScaleOutput->GetValue();
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
	this->m_config->useGifInsteadImage = !this->m_config->useGifInsteadImage;
	ev.Skip();
}

void cPanelProgramConfig::comboGifQuality_Select(wxCommandEvent& ev) {
	wxString val = ev.GetString();
	this->m_config->gifResizePercentage = Utils::GifResizePercentageFromString(val.ToStdString());
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

void cPanelProgramConfig::chkShowIgnoredAreas_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->showIgnoredAreas = !this->m_config->showIgnoredAreas;
	ev.Skip();
}