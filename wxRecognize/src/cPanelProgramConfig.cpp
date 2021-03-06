#include "cPanelProgramConfig.hpp"

#define BIND(Widget_, EventTag_, EventHandler_, Function_) Widget_->Bind(EventTag_, \
																			[this](EventHandler_& ev) { \
																				this->m_sharedData->btnApplyChanges->Enable(true); \
																				Function_(ev); \
																			})

cPanelProgramConfig::cPanelProgramConfig(wxBookCtrlBase* parent, ProgramConfiguration& progConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(&progConfig), m_sharedData(sharedData) {
				
	this->m_detectionsMethods.Add(_("HOG Descriptor (opencv) person only"));
	this->m_detectionsMethods.Add(_("Yolo DarkNet V4 (AlexeyAB) objects detections"));
	
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerLeft = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);

	int imax = std::numeric_limits<int>::max();
	double dmax = std::numeric_limits<double>::max();
	
	m_book = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_LEFT);
	
	const int flags = wxALL | wxGROW;
	const int border = 7;
	
	// ---------
	//  General
	// ---------
	
	wxPanel* panelGeneral = new wxPanel(m_book, wxID_ANY);
	wxSizer* sizerGeneral = new wxBoxSizer(wxVERTICAL);
	
	this->m_spinMsBetweenFrames = new wxSpinCtrl(panelGeneral, PROGRAM_ids::SPIN_MsBetweenFrames, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->msBetweenFrame);
	BIND(m_spinMsBetweenFrames, wxEVT_SPINCTRL, wxSpinEvent, spinMsBetweenFrames_SpinChange);
	
	this->m_spinMsBetweenFramesAfterChange = new wxSpinCtrl(panelGeneral, PROGRAM_ids::SPIN_MsBetweenFrames, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->msBetweenFrameAfterChange);
	BIND(m_spinMsBetweenFramesAfterChange, wxEVT_SPINCTRL, wxSpinEvent, spinMsBetweenFrames_SpinChange);
							
	this->m_spinRatioScaleOutput = new wxSpinCtrlDouble(panelGeneral, PROGRAM_ids::SPIN_RatioScaleOutput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->ratioScaleOutput);
	this->m_spinRatioScaleOutput->SetDigits(2);
	BIND(m_spinRatioScaleOutput, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinRatioScaleOutput_SpinChange);

	sizerGeneral->Add(WidgetsHelper::JoinWidgetsOnSizerH(
					WidgetsHelper::GetSizerItemLabel(panelGeneral, m_spinMsBetweenFrames, _("Milliseconds between frame"), _("Greater this value = Lower CPU usage.\n FPS = 1000 / ms, e.g. 25 ms = 40 FPS.\n Try different values and determine the efficiency vs effectiveness.")), 
					WidgetsHelper::GetSizerItemLabel(panelGeneral, m_spinRatioScaleOutput, _("Ratio Scale output"), _("Scales the output")), 
					5
				), 0, flags, border);
				
	sizerGeneral->Add(WidgetsHelper::GetSizerItemLabel(
			panelGeneral,
			m_spinMsBetweenFramesAfterChange,
			_("ms between frames after change"),
			_("Same as ms Between Frame. But only will be applied after detecting a change in the frames.")
		), 0, flags, border);

	this->m_chkShowPreviewCameras = new wxCheckBox(panelGeneral, PROGRAM_ids::CHK_ShowPreviewCameras, _("Show preview of the cameras"));
	this->m_chkShowPreviewCameras->SetValue(m_config->showPreview);
	BIND(m_chkShowPreviewCameras, wxEVT_CHECKBOX, wxCommandEvent, chkShowPreviewCameras_CheckBoxClick);
	sizerGeneral->Add(m_chkShowPreviewCameras, 0, flags, border);
	
	this->m_chkShowAreaCameraSees = new wxCheckBox(panelGeneral, PROGRAM_ids::CHK_ShowAreaCameraSees, _("Show the cameras ROI"));
	this->m_chkShowAreaCameraSees->SetToolTip(_("Draws the rectangle cooresponding to the region of interest of each camera."));
	this->m_chkShowAreaCameraSees->SetValue(m_config->showAreaCameraSees);
	BIND(m_chkShowAreaCameraSees, wxEVT_CHECKBOX, wxCommandEvent, chkShowAreaCameraSees_CheckBoxClick);	
	sizerGeneral->Add(m_chkShowAreaCameraSees, 0, flags, border);
	
	this->m_chkShowProcessedImages = new wxCheckBox(panelGeneral, PROGRAM_ids::CHK_ShowProcessedImages, _("Show processed images"));
	this->m_chkShowProcessedImages->SetToolTip(_("Shows frames cropped rotated and with the change between them."));
	this->m_chkShowProcessedImages->SetValue(m_config->showProcessedFrames);
	BIND(m_chkShowProcessedImages, wxEVT_CHECKBOX, wxCommandEvent, chkShowProcessedImages_CheckBoxClick);
	sizerGeneral->Add(m_chkShowProcessedImages, 0, flags, border);

	this->m_chkShowIgnoredAreas = new wxCheckBox(panelGeneral, wxID_ANY, _("Show ignored areas"));
	this->m_chkShowIgnoredAreas->SetToolTip(_("Draws the ignored area for each camera"));
	this->m_chkShowIgnoredAreas->SetValue(m_config->showIgnoredAreas);
	BIND(m_chkShowIgnoredAreas, wxEVT_CHECKBOX, wxCommandEvent, chkShowIgnoredAreas_CheckBoxClick);	
	sizerGeneral->Add(m_chkShowIgnoredAreas, 0, flags, border);
	
	this->m_comboDetectionMethod = new wxComboBox(panelGeneral, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_detectionsMethods);
	this->m_comboDetectionMethod->SetToolTip(_("Selects the method or algorithm wich will be used to detect objects in the images after a change."));
	this->m_comboDetectionMethod->SetValue(m_detectionsMethods[this->m_config->detectionMethod]);
	BIND(m_comboDetectionMethod, wxEVT_COMBOBOX, wxCommandEvent, comboDetectionMethod_Select);
	wxSizer* szDetectionMethod = new wxBoxSizer(wxHORIZONTAL);
	szDetectionMethod->Add(WidgetsHelper::GetSizerItemLabel(panelGeneral, m_comboDetectionMethod, _("Detection method")),  3, wxRIGHT | wxGROW, 5);
	sizerGeneral->Add(szDetectionMethod, 0, flags, border);
	
	this->m_btnOpenSettingsMethod = new wxButton(panelGeneral, wxID_ANY, _("Settings"), wxDefaultPosition, wxSize(10, 25));
	BIND(m_btnOpenSettingsMethod, wxEVT_BUTTON, wxCommandEvent, btnOpenSettingsMethod_Click);
	szDetectionMethod->Add(m_btnOpenSettingsMethod, 1, wxTOP | wxGROW, 15);

	// --------------
	//  Notification
	// --------------
	
	wxPanel* panelNotification = new wxPanel(m_book, wxID_ANY);
	wxSizer* sizerNotification = new wxBoxSizer(wxVERTICAL);
	
	this->m_chkUseTelegramBot = new wxCheckBox(panelNotification, PROGRAM_ids::CHK_UseTelegramBot, _("Use telegram bot"));
	this->m_chkUseTelegramBot->SetToolTip(_("Disabling this the program will not send notifications."));
	this->m_chkUseTelegramBot->SetValue(m_config->telegramConfig.useTelegramBot);
	BIND(m_chkUseTelegramBot, wxEVT_CHECKBOX, wxCommandEvent, chkUseTelegramBot_CheckBoxClick);	
	sizerNotification->Add(m_chkUseTelegramBot, 0, flags, border);
	
	this->m_chkSendImageMessageAfterChange = new wxCheckBox(panelNotification, PROGRAM_ids::CHK_SendImageAfterDetectigChange, _("Send image message after detectig change"));
	this->m_chkSendImageMessageAfterChange->SetValue(m_config->sendImageWhenDetectChange);
	BIND(m_chkSendImageMessageAfterChange, wxEVT_CHECKBOX, wxCommandEvent, chkSendImageAfterDetectigChange_CheckBoxClick);
	
	this->m_chkSendTextMessageAfterChange = new wxCheckBox(panelNotification, wxID_ANY, _("Send text message after detectig change"));
	this->m_chkSendTextMessageAfterChange->SetValue(m_config->sendTextWhenDetectChange);
	BIND(m_chkSendTextMessageAfterChange, wxEVT_CHECKBOX, wxCommandEvent, chkSendTextMessageAfterChange_CheckBoxClick);
	sizerNotification->Add(WidgetsHelper::JoinWidgetsOnSizerH(
								m_chkSendImageMessageAfterChange, 
								m_chkSendTextMessageAfterChange,
								5), 0, flags, border);

	this->m_chkSendImageOfAllCameras = new wxCheckBox(panelNotification, PROGRAM_ids::CHK_SendImageOfAllCameras, _("Send image of all cameras"));
	this->m_chkSendImageOfAllCameras->SetToolTip(_("Sends a image of all the cameras after detecting a change. Does not replace send image or send text after change."));
	this->m_chkSendImageOfAllCameras->SetValue(m_config->sendImageOfAllCameras);
	BIND(m_chkSendImageOfAllCameras, wxEVT_CHECKBOX, wxCommandEvent, chkSendImageOfAllCameras_CheckBoxClick);
	sizerNotification->Add(m_chkSendImageOfAllCameras, 0, flags, border);

	this->m_chkUseGifInsteadOfImage = new wxCheckBox(panelNotification, PROGRAM_ids::CHK_UseGifInsteadOfImage, _("Use gif instead of image"));
	this->m_chkUseGifInsteadOfImage->SetToolTip(_("If not cheked, the program will send a image (if send image after change is cheked).\nElse it'll send a gif with the ammount of frames that you specify in framesBefore, framesAfter"));
	this->m_chkUseGifInsteadOfImage->SetValue(m_config->useGifInsteadImage);
	BIND(m_chkUseGifInsteadOfImage, wxEVT_CHECKBOX, wxCommandEvent, chkUseGifInsteadOfImage_CheckBoxClick);
	sizerNotification->Add(m_chkUseGifInsteadOfImage, 0, flags, border);
	
	this->m_spinSecondsBetweenImage = new wxSpinCtrl(panelNotification, PROGRAM_ids::SPIN_SecondsBetweenImage, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->secondsBetweenImage);
	BIND(m_spinSecondsBetweenImage, wxEVT_SPINCTRL, wxSpinEvent, spinSecondsBetweenImage_SpinChange);
//	sizerNotification->Add(, 0, flags, border);
	
	this->m_spinSecondsBetweenMessage = new wxSpinCtrl(panelNotification, PROGRAM_ids::SPIN_SecondsBetweenMessage, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->secondsBetweenMessage);
	BIND(m_spinSecondsBetweenMessage, wxEVT_SPINCTRL, wxSpinEvent, spinSecondsBetweenMessage_SpinChange);
	sizerNotification->Add(WidgetsHelper::JoinWidgetsOnSizerH(
								WidgetsHelper::GetSizerItemLabel(panelNotification, m_spinSecondsBetweenImage, _("Seconds between message with image")), 
								WidgetsHelper::GetSizerItemLabel(panelNotification, m_spinSecondsBetweenMessage, _("Seconds between message with text")),
								5), 0, flags, border);
	
	this->m_spinGifResizeLevel = new wxSpinCtrl(panelNotification, PROGRAM_ids::COMBO_GifQuality, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, this->m_config->gifResizePercentage);
	this->m_spinGifResizeLevel->SetToolTip(_("Sets the resize level of the gif. While higher, more lightweigth the gif.\nSet to 0 leaves the original size and at 50 it resizes it in half the size."));
	BIND(m_spinGifResizeLevel, wxEVT_SPINCTRL, wxSpinEvent, spinGifQuality_SpinChange);
	sizerNotification->Add(WidgetsHelper::GetSizerItemLabel(panelNotification, m_spinGifResizeLevel, _("Gif resize level")), 0, flags, border);
	
	this->m_spinFramesBefore = new wxSpinCtrl(panelNotification, PROGRAM_ids::SPIN_FramesBefore, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->numberGifFrames.framesBefore);
	BIND(m_spinFramesBefore, wxEVT_SPINCTRL, wxSpinEvent, spinFramesBefore_SpinChange);
	
	this->m_spinFramesAfter = new wxSpinCtrl(panelNotification, PROGRAM_ids::SPIN_FramesAfter, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->numberGifFrames.framesAfter);
	BIND(m_spinFramesAfter, wxEVT_SPINCTRL, wxSpinEvent, spinFramesAfter_SpinChange);
	sizerNotification->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(panelNotification, m_spinFramesBefore, _("Frames Before"), _("Ammount of frames to store before the change.")),
						WidgetsHelper::GetSizerItemLabel(panelNotification, m_spinFramesAfter, _("Frames After"), _("Ammount of frames to store after the change.")),
						5
					), 0, flags, border);

	// ----------
	//  Telegram
	// ----------
		
	wxPanel* panelTelegram = new wxPanel(m_book, wxID_ANY);
	wxSizer* sizerTelegram = new wxBoxSizer(wxVERTICAL);
	
	this->m_txtTelegramBotApiKey = new wxTextCtrl(panelTelegram, PROGRAM_ids::TXT_TelegramBotApiKey, m_config->telegramConfig.apiKey, wxDefaultPosition, wxDefaultSize);
	this->m_txtTelegramBotApiKey->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtTelegramBotApiKey_KillFocus, this);
	BIND(m_txtTelegramBotApiKey, wxEVT_KILL_FOCUS, wxFocusEvent, txtTelegramBotApiKey_KillFocus);
	sizerTelegram->Add(WidgetsHelper::GetSizerItemLabel(panelTelegram, m_txtTelegramBotApiKey, _("Telegram bot api key")), 0, flags, border);

	this->m_txtTelegramChatId = new wxTextCtrl(panelTelegram, PROGRAM_ids::TXT_TelegramChatId, m_config->telegramConfig.chatId, wxDefaultPosition, wxDefaultSize);
	BIND(m_txtTelegramChatId, wxEVT_KILL_FOCUS, wxFocusEvent, txtTelegramChatId_KillFocus);
	sizerTelegram->Add(WidgetsHelper::GetSizerItemLabel(panelTelegram, m_txtTelegramChatId, _("Telegram bot chat id")), 0, flags, border);

	this->m_txtAuthUsersToSendActions = new wxTextCtrl(panelTelegram, PROGRAM_ids::TXT_AuthUsersToSendActions, Utils::VectorToCommaString(this->m_config->authUsersToSendActions), wxDefaultPosition, wxDefaultSize);
	this->m_txtAuthUsersToSendActions->Bind(wxEVT_KILL_FOCUS, &cPanelProgramConfig::txtAuthUsersToSendActions_KillFocus, this);
	BIND(m_txtAuthUsersToSendActions, wxEVT_KILL_FOCUS, wxFocusEvent, txtAuthUsersToSendActions_KillFocus);
	sizerTelegram->Add(WidgetsHelper::GetSizerItemLabel(panelTelegram, m_txtAuthUsersToSendActions, _("Users athorized to send actions")), 0, flags, border);

	panelGeneral->SetSizer(sizerGeneral);
	panelNotification->SetSizer(sizerNotification);
	panelTelegram->SetSizer(sizerTelegram);
		
	m_book->AddPage(panelGeneral, _("General"));
	
	m_book->AddPage(panelNotification, _("Notification"));
	
	m_book->AddPage(panelTelegram, _("Telegram"));
	
	sizer->Add(m_book, 1, wxGROW);
	
	this->EnableDisableControlsBotGif();

	this->SetSizer(sizer);
}

cPanelProgramConfig::~cPanelProgramConfig() { }

void cPanelProgramConfig::spinMsBetweenFrames_SpinChange(wxSpinEvent& ev) {
	this->m_config->msBetweenFrame = this->m_spinMsBetweenFrames->GetValue();
	ev.Skip();
}

void cPanelProgramConfig::spinMsBetweenFramesAfterChange_SpinChange(wxSpinEvent& ev) {
	this->m_config->msBetweenFrameAfterChange = this->m_spinMsBetweenFramesAfterChange->GetValue();
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
	
	this->EnableDisableControlsBotGif();
	
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
	
	this->EnableDisableControlsBotGif();
	
	ev.Skip();
}

void cPanelProgramConfig::spinGifQuality_SpinChange(wxSpinEvent& ev) {
	this->m_config->gifResizePercentage = this->m_spinGifResizeLevel->GetValue();
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

void cPanelProgramConfig::chkSendTextMessageAfterChange_CheckBoxClick(wxCommandEvent& ev) {
	this->m_config->sendTextWhenDetectChange = !this->m_config->sendTextWhenDetectChange;
	ev.Skip();
}

void cPanelProgramConfig::EnableDisableControlsBotGif() {
	this->m_spinFramesBefore->Enable(this->m_config->telegramConfig.useTelegramBot && this->m_config->useGifInsteadImage);
	this->m_spinFramesAfter->Enable(this->m_config->telegramConfig.useTelegramBot && this->m_config->useGifInsteadImage);
	this->m_spinGifResizeLevel->Enable(this->m_config->telegramConfig.useTelegramBot && this->m_config->useGifInsteadImage);
	
	this->m_txtAuthUsersToSendActions->Enable(this->m_config->telegramConfig.useTelegramBot);
	this->m_txtTelegramBotApiKey->Enable(this->m_config->telegramConfig.useTelegramBot);
	this->m_txtTelegramChatId->Enable(this->m_config->telegramConfig.useTelegramBot);
	
	this->m_spinSecondsBetweenImage->Enable(this->m_config->telegramConfig.useTelegramBot);
	this->m_spinSecondsBetweenMessage->Enable(this->m_config->telegramConfig.useTelegramBot);
}

void cPanelProgramConfig::comboDetectionMethod_Select(wxCommandEvent& ev) {
	this->m_config->detectionMethod = (DetectionMethod)this->m_comboDetectionMethod->GetSelection();
	ev.Skip();
}

void cPanelProgramConfig::btnOpenSettingsMethod_Click(wxCommandEvent& ev) {
//	wxSize sz = this->GetSize();
//	sz.SetWidth(sz.GetWidth() / 2);
//	
//	cDialogConfigureDetection* dialog = new cDialogConfigureDetection(this, wxID_ANY, _("Settings"), wxDefaultPosition, sz);
//	dialog->Show(true);
}