#include "cPanelCameraConfig.hpp"

wxBEGIN_EVENT_TABLE(cPanelCameraConfig, wxPanel)
	EVT_SPINCTRL(CAMERA_ids::SPIN_order, cPanelCameraConfig::spinOrder_Change)
	EVT_SPINCTRL(CAMERA_ids::SPIN_rotation, cPanelCameraConfig::spinRotation_Change)
	EVT_SPINCTRL(CAMERA_ids::SPIN_changeThreshold, cPanelCameraConfig::spinChangeThreshold_Change)
	EVT_SPINCTRL(CAMERA_ids::SPIN_minThreshold, cPanelCameraConfig::spinMinThreshold_Change)
	EVT_SPINCTRLDOUBLE(CAMERA_ids::SPIN_increaseThreshFactor, cPanelCameraConfig::spinIncreaseThreshold_Change)
	EVT_SPINCTRL(CAMERA_ids::SPIN_updateThresFreq, cPanelCameraConfig::spinUpdateThresholdFrequency_Change)
	EVT_COMBOBOX(CAMERA_ids::COMBO_type, cPanelCameraConfig::comboType_Select)
	EVT_SPINCTRLDOUBLE(CAMERA_ids::SPIN_hitThreshold, cPanelCameraConfig::spinHitThreshold_Change)
	EVT_SPINCTRLDOUBLE(CAMERA_ids::SPIN_noiseThresh, cPanelCameraConfig::spinNoiseThreshold_Change)
	EVT_BUTTON(CAMERA_ids::BTN_selectRoi, cPanelCameraConfig::btnSelectRoi_Click)
	EVT_BUTTON(CAMERA_ids::BTN_selectIgnoredAreas, cPanelCameraConfig::btnSelectIgnoredAreas_Click)
	EVT_SPINCTRL(CAMERA_ids::SPIN_FramesAnalyzeBefore, cPanelCameraConfig::spinFramesAnalyzeBefore_Change)
	EVT_SPINCTRL(CAMERA_ids::SPIN_FramesAnalyzeAfter, cPanelCameraConfig::spinFramesAnalyzeAfter_Change)
wxEND_EVENT_TABLE()

cPanelCameraConfig::cPanelCameraConfig(wxBookCtrlBase* parent, CameraConfiguration* camConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(camConfig), m_sharedData(sharedData) {
	
	int imax = std::numeric_limits<int>::max();
	double dmax = std::numeric_limits<double>::max();
	
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerLeft = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerRight = new wxBoxSizer(wxVERTICAL);
	
	m_txtName = new wxTextCtrl(this, CAMERA_ids::TXT_name, m_config->cameraName, wxDefaultPosition, wxDefaultSize);
	this->m_txtName->Bind(wxEVT_KILL_FOCUS, &cPanelCameraConfig::txtName_KillFocus, this);

	m_txtUrl = new wxTextCtrl(this, CAMERA_ids::TXT_url, m_config->url, wxDefaultPosition, wxDefaultSize);
	this->m_txtUrl->Bind(wxEVT_KILL_FOCUS, &cPanelCameraConfig::txtUrl_KillFocus, this);
	
	m_spinOrder = new wxSpinCtrl(this, CAMERA_ids::SPIN_order, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->order);

	m_spinRotation = new wxSpinCtrl(this, CAMERA_ids::SPIN_rotation, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -360, 360, m_config->rotation);

	m_spinChangeThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_changeThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->changeThreshold);
	
	m_spinMinimumThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_minThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->minimumThreshold);
	
	m_spinIncreaseThresholdFactor = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_increaseThreshFactor, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->increaseTresholdFactor);
	m_spinIncreaseThresholdFactor->SetDigits(2);
	
	m_spinUpdateThresholdFreq = new wxSpinCtrl(this, CAMERA_ids::SPIN_updateThresFreq, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->updateThresholdFrequency);

	m_comboType = new wxComboBox(this, CAMERA_ids::COMBO_type, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	m_comboType->Append(wxString("Active"));
	m_comboType->Append(wxString("Disabled"));
	m_comboType->Append(wxString("Sentry"));
	m_comboType->SetValue((this->m_config->type == CAMERA_DISABLED ? "Disabled" : (this->m_config->type == CAMERA_SENTRY ? "Sentry"  : "Active")));

	m_spinHitThreshold = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_hitThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->hitThreshold);
	m_spinHitThreshold->SetDigits(2);

	m_spinNoise = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_noiseThresh, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->noiseThreshold);
	m_spinNoise->SetDigits(2);
	
	m_btnSelectRoi = new wxButton(this, CAMERA_ids::BTN_selectRoi, "Select Region of interest");
	
	m_btnSelectIgnoredAreas = new wxButton(this, CAMERA_ids::BTN_selectIgnoredAreas, "Select ignored areas");
	
	this->m_spinFramesAnalyzeBefore = new wxSpinCtrl(this, CAMERA_ids::SPIN_FramesAnalyzeBefore, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->framesToAnalyze.framesBefore);
	this->m_spinFramesAnalyzeAfter = new wxSpinCtrl(this, CAMERA_ids::SPIN_FramesAnalyzeAfter, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, this->m_config->framesToAnalyze.framesAfter);
	
	// --------------- Sizers
	const int flags = wxALL | wxEXPAND;
	const int border = 10;
	
	// Add widgets to sizer
	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtName, "Name"), 0, flags, border);

	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtUrl, "Url"), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_comboType, "Camera type"), 0, flags, border);

//	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtUrl, "Url"), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinOrder, "Order"),
						WidgetsHelper::GetSizerItemLabel(this, m_spinRotation, "Rotation"),
						5
					), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinChangeThreshold, "Change Threshold"), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						m_btnSelectRoi,
						m_btnSelectIgnoredAreas,
						5
					), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinMinimumThreshold, "Minimum Threshold"), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinIncreaseThresholdFactor, "Increase threshold factor"), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinUpdateThresholdFreq, "Update threshold frequency"), 0, flags, border);

	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinHitThreshold, "Hit threshold"),
						WidgetsHelper::GetSizerItemLabel(this, m_spinNoise, "Noise threshold"),
						5
					), 0, flags, border);
	
	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAnalyzeBefore, "Frames Analyze Before"),
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAnalyzeAfter, "Frames Analyze After"),
						5
					), 0, flags, border);
		
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

void cPanelCameraConfig::spinChangeThreshold_Change(wxSpinEvent& ev) {
	this->m_config->changeThreshold = this->m_spinChangeThreshold->GetValue();
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
	wxString t = this->m_comboType->GetValue();
	this->m_config->type = (t == "Disabled" ? 0 : (t == wxT("Sentry")  ? 1 : 2));
}

void cPanelCameraConfig::btnSelectRoi_Click(wxCommandEvent& ev) {
	AreaSelector::SelectCameraROI(this->m_config->url, this->m_config->roi);
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