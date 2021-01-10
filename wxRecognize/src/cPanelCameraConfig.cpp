#include "cPanelCameraConfig.hpp"

#define BIND(Widget_, EventTag_, EventHandler_, Function_) Widget_->Bind(EventTag_, \
																			[this](EventHandler_& ev) { \
																				this->m_sharedData->btnApplyChanges->Enable(true); \
																				Function_(ev); \
																			})

cPanelCameraConfig::cPanelCameraConfig(wxBookCtrlBase* parent, CameraConfiguration* camConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(camConfig), m_sharedData(sharedData) {
	
	m_comboTypeChoices.Add(_("Disabled"));
	m_comboTypeChoices.Add(_("Sentry"));
	m_comboTypeChoices.Add(_("Active"));

	int imax = std::numeric_limits<int>::max();
	double dmax = std::numeric_limits<double>::max();
	
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerLeft = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);
	
	m_txtName = new wxTextCtrl(this, CAMERA_ids::TXT_name, m_config->cameraName, wxDefaultPosition, wxDefaultSize);
	BIND(m_txtName, wxEVT_KILL_FOCUS, wxFocusEvent, txtName_KillFocus);
	
	m_txtUrl = new wxTextCtrl(this, CAMERA_ids::TXT_url, m_config->url, wxDefaultPosition, wxDefaultSize);
	BIND(m_txtUrl, wxEVT_KILL_FOCUS, wxFocusEvent, txtUrl_KillFocus);
	
	m_spinOrder = new wxSpinCtrl(this, CAMERA_ids::SPIN_order, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->order);
	BIND(m_spinOrder, wxEVT_SPINCTRL, wxSpinEvent, spinOrder_Change);

	m_spinRotation = new wxSpinCtrl(this, CAMERA_ids::SPIN_rotation, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -360, 360, m_config->rotation);
	BIND(m_spinRotation, wxEVT_SPINCTRL, wxSpinEvent, spinRotation_Change);

	m_spinMinimumThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_minThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->minimumThreshold);
	BIND(m_spinMinimumThreshold, wxEVT_SPINCTRL, wxSpinEvent, spinMinThreshold_Change);
	
	m_spinIncreaseThresholdFactor = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_increaseThreshFactor, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->increaseTresholdFactor);
	m_spinIncreaseThresholdFactor->SetDigits(2);
	BIND(m_spinIncreaseThresholdFactor, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinIncreaseThreshold_Change);
	
	m_spinUpdateThresholdFreq = new wxSpinCtrl(this, CAMERA_ids::SPIN_updateThresFreq, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->updateThresholdFrequency);
	BIND(m_spinUpdateThresholdFreq, wxEVT_SPINCTRL, wxSpinEvent, spinUpdateThresholdFrequency_Change);
	
	m_comboType = new wxComboBox(this, CAMERA_ids::COMBO_type, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_comboTypeChoices);
	m_comboType->SetValue(m_comboTypeChoices[this->m_config->type]);
	BIND(m_comboType, wxEVT_COMBOBOX, wxCommandEvent, comboType_Select);

	m_spinHitThreshold = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_hitThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->hitThreshold);
	m_spinHitThreshold->SetDigits(2);
	BIND(m_spinHitThreshold, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinHitThreshold_Change);
	
	m_spinNoise = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_noiseThresh, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->noiseThreshold);
	m_spinNoise->SetDigits(2);
	BIND(m_spinNoise, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinNoiseThreshold_Change);
	
	m_btnSelectRoi = new wxButton(this, CAMERA_ids::BTN_selectRoi, _("Select Region of interest"));
	BIND(m_btnSelectRoi, wxEVT_BUTTON, wxCommandEvent, btnSelectRoi_Click);
	
	m_btnSelectIgnoredAreas = new wxButton(this, CAMERA_ids::BTN_selectIgnoredAreas, _("Select ignored areas"));
	BIND(m_btnSelectIgnoredAreas, wxEVT_BUTTON, wxCommandEvent, btnSelectIgnoredAreas_Click);
	
	this->m_spinFramesAnalyzeBefore = new wxSpinCtrl(this, CAMERA_ids::SPIN_FramesAnalyzeBefore, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->framesToAnalyze.framesBefore);
	BIND(m_spinFramesAnalyzeBefore, wxEVT_SPINCTRL, wxSpinEvent, spinFramesAnalyzeBefore_Change);
	
	this->m_spinFramesAnalyzeAfter = new wxSpinCtrl(this, CAMERA_ids::SPIN_FramesAnalyzeAfter, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->framesToAnalyze.framesAfter);
	BIND(m_spinFramesAnalyzeAfter, wxEVT_SPINCTRL, wxSpinEvent, spinFramesAnalyzeAfter_Change);
			
	this->m_spinMinPercentageAreaIgnored = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, this->m_config->minPercentageAreaNeededToIgnore);
	BIND(m_spinMinPercentageAreaIgnored, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinMinPercentageAreaIgnored_Change);
	
	// --------------- Sizers
	const int flags = wxALL | wxEXPAND;
	const int border = 10;
	
	// Add widgets to sizer
	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtName, _("Name"), _("Name of the camera")), 0, flags, border);

	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtUrl, _("Url"), _("Url of the camera")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_comboType, _("Camera type"), _("Disabled: Camera is disabled, doesn't show or process frames.\nSentry: Only sends notifications.\nActive: Same as Sentry but try to recognize a person in the frames selected on 'framesToAnalyze'.")), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinOrder, _("Order"), _("Position of the camera in the preview. Starts at 0")),
						WidgetsHelper::GetSizerItemLabel(this, m_spinRotation, _("Rotation"), _("Rotation of the camera, helps to detect objects correctly")),
						5
					), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						m_btnSelectRoi,
						m_btnSelectIgnoredAreas,
						5
					), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinMinimumThreshold, _("Minimum Threshold"), _("Minimum number of different pixels between the last 2 frames. Is used to leave a margin to not trigger the alert constantly.\nIs recommended to set it at a low number, like 10. You may will have to change it if you change the theshold noise or update Frequency")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinIncreaseThresholdFactor, _("Increase threshold factor"), _("Since the program is calculating the average change of pixels between the last two frames you need to leave a margin\n to avoid sending notifications over small or insignificant changes. \nA general good value is between 1.04 (4%) and 1.30 (30%) of the average change.\nAlso you could leave this to 1 if with 'minimun theshold' it's working fine.")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinUpdateThresholdFreq, _("Update threshold frequency (seconds)"), _("This tells the program how frequent (seconds) to update the ammount of pixels changed between the last two frames needed to trigger to alert. On camera where there is fast changing objects is good to leave this value low, e.g. 5.")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinHitThreshold, _("Hit threshold"), "?"),
						WidgetsHelper::GetSizerItemLabel(this, m_spinNoise, _("Noise threshold"), _("This value helps deleting noise from the image, so more noise in each frame higher this value should be. Average values goes from 30 to 45.")),
						5
					), 0, flags, border);
	
	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAnalyzeBefore, _("Frames Analyze Before"), _("Used to know how much frames search for a person in the frames of the 'before' buffer. Used only if 'use gif instead of image' is cheked.\nShould be a number less than or equal than 'frames before' from the Program Configuration.")),
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAnalyzeAfter, _("Frames Analyze After"), _("Used to know how much frames search for a person in the frames of the 'after' buffer. Used only if 'use gif instead of image' is cheked.\nShould be a number less than or equal than 'frames after' from the Program Configuration.")),
						5
					), 0, flags, border);
	
	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinMinPercentageAreaIgnored, _("Minimum percentage of area"), _("Percentage of the area of the rectangle that describes the change from the resulting area of the intersection of that rectangle with each ignored area.\nUsed to leave a margin of error when detecting changes that occur within the ignored areas.")), 0, flags, border);
		
	sizer->Add(sizerLeft, 2, wxGROW, 0);
	sizer->Add(sizerRight, 2, wxGROW, 0);

	this->SetSizer(sizer);
}

cPanelCameraConfig::~cPanelCameraConfig() {

}

void cPanelCameraConfig::spinOrder_Change(wxSpinEvent& ev) {
	this->m_config->order = this->m_spinOrder->GetValue();
}

void cPanelCameraConfig::spinRotation_Change(wxSpinEvent& ev) {
	this->m_config->rotation = this->m_spinRotation->GetValue();
}

void cPanelCameraConfig::spinMinThreshold_Change(wxSpinEvent& ev) {
	this->m_config->minimumThreshold = this->m_spinMinimumThreshold->GetValue();
}

void cPanelCameraConfig::spinIncreaseThreshold_Change(wxSpinDoubleEvent& ev) {
	this->m_config->increaseTresholdFactor = this->m_spinIncreaseThresholdFactor->GetValue();
}

void cPanelCameraConfig::spinUpdateThresholdFrequency_Change(wxSpinEvent& ev) {
	this->m_config->updateThresholdFrequency = this->m_spinUpdateThresholdFreq->GetValue();
}

void cPanelCameraConfig::txtName_KillFocus(wxFocusEvent& ev) {
	this->m_config->cameraName = this->m_txtName->GetValue();
}

void cPanelCameraConfig::txtUrl_KillFocus(wxFocusEvent& ev) {
	this->m_config->url = this->m_txtUrl->GetValue();
}

void cPanelCameraConfig::spinHitThreshold_Change(wxSpinDoubleEvent& ev) {
	this->m_config->hitThreshold = this->m_spinHitThreshold->GetValue();
}

void cPanelCameraConfig::spinNoiseThreshold_Change(wxSpinDoubleEvent& ev) {
	this->m_config->noiseThreshold = this->m_spinNoise->GetValue();
}

void cPanelCameraConfig::comboType_Select(wxCommandEvent& ev) {
	this->m_config->type = (CAMERATYPE) this->m_comboType->GetSelection();
}

void cPanelCameraConfig::btnSelectRoi_Click(wxCommandEvent& ev) {
	AreaSelector::SelectCameraROI(*this->m_config);
}

void cPanelCameraConfig::btnSelectIgnoredAreas_Click(wxCommandEvent& ev) {
	AreaSelector::SelectCameraIgnoredAreas(*this->m_config);
}

void cPanelCameraConfig::spinFramesAnalyzeBefore_Change(wxSpinEvent& ev) {
	this->m_config->framesToAnalyze.framesBefore = this->m_spinFramesAnalyzeBefore->GetValue();
}

void cPanelCameraConfig::spinFramesAnalyzeAfter_Change(wxSpinEvent& ev) {
	this->m_config->framesToAnalyze.framesAfter = this->m_spinFramesAnalyzeAfter->GetValue();
}

void cPanelCameraConfig::spinMinPercentageAreaIgnored_Change(wxSpinDoubleEvent& ev) {
	this->m_config->minPercentageAreaNeededToIgnore = this->m_spinMinPercentageAreaIgnored->GetValue();
}