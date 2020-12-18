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
wxEND_EVENT_TABLE()

cPanelCameraConfig::cPanelCameraConfig(wxBookCtrlBase* parent, CameraConfiguration* camConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(camConfig), m_sharedData(sharedData) {
	
	int imax = std::numeric_limits<int>::max();
	double dmax = std::numeric_limits<double>::max();
	
	wxSizer* sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Tweak configuration");
	wxSizer* sizerLeft = new wxStaticBoxSizer(wxVERTICAL, this, "");
	wxSizer* sizerRight = new wxStaticBoxSizer(wxVERTICAL, this, "");
	
	wxStaticText* labelName = new wxStaticText(this, wxID_ANY, "Name", wxDefaultPosition, wxDefaultSize);	
	m_txtName = new wxTextCtrl(this, CAMERA_ids::TXT_name, m_config->cameraName, wxDefaultPosition, wxDefaultSize);
	this->m_txtName->Bind(wxEVT_KILL_FOCUS, &cPanelCameraConfig::txtName_KillFocus, this);

	wxStaticText* labelUrl = new wxStaticText(this, wxID_ANY, "Url", wxDefaultPosition, wxDefaultSize);
	m_txtUrl = new wxTextCtrl(this, CAMERA_ids::TXT_url, m_config->url, wxDefaultPosition, wxDefaultSize);
	this->m_txtUrl->Bind(wxEVT_KILL_FOCUS, &cPanelCameraConfig::txtUrl_KillFocus, this);
	
	wxStaticText* labelOrder = new wxStaticText(this, wxID_ANY, "Order", wxDefaultPosition, wxDefaultSize);
	m_spinOrder = new wxSpinCtrl(this, CAMERA_ids::SPIN_order, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->order);

	wxStaticText* labelRotation = new wxStaticText(this, wxID_ANY, "Rotation", wxDefaultPosition, wxDefaultSize);
	m_spinRotation = new wxSpinCtrl(this, CAMERA_ids::SPIN_rotation, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -360, 360, m_config->rotation);

	wxStaticText* labelChangeThreshold = new wxStaticText(this, wxID_ANY, "Change threshold", wxDefaultPosition, wxDefaultSize);
	m_spinChangeThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_changeThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->changeThreshold);
	
	wxStaticText* labelMinimumThreshold = new wxStaticText(this, wxID_ANY, "Minimum threshold", wxDefaultPosition, wxDefaultSize);
	m_spinMinimumThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_minThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->minimumThreshold);
	
	wxStaticText* labelIncreaseThresholdFactor = new wxStaticText(this, wxID_ANY, "Increase threshold factor", wxDefaultPosition, wxDefaultSize);
	m_spinIncreaseThresholdFactor = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_increaseThreshFactor, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->increaseTresholdFactor);
	m_spinIncreaseThresholdFactor->SetDigits(2);
	
	wxStaticText* labelUpdateThresholdFreq = new wxStaticText(this, wxID_ANY, "Update threshold frequency (seconds)", wxDefaultPosition, wxDefaultSize);
	m_spinUpdateThresholdFreq = new wxSpinCtrl(this, CAMERA_ids::SPIN_updateThresFreq, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->updateThresholdFrequency);

	wxStaticText* labelComboType = new wxStaticText(this, wxID_ANY, "Camera type", wxDefaultPosition, wxDefaultSize);
	m_comboType = new wxComboBox(this, CAMERA_ids::COMBO_type, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	m_comboType->Append(wxString("Active"));
	m_comboType->Append(wxString("Disabled"));
	m_comboType->Append(wxString("Sentry"));
	m_comboType->SetValue((this->m_config->type == CAMERA_DISABLED ? "Disabled" : (this->m_config->type == CAMERA_SENTRY ? "Sentry"  : "Active")));

	wxStaticText* labelHitThreshold = new wxStaticText(this, wxID_ANY, "Hit threshold", wxDefaultPosition, wxDefaultSize);
	m_spinHitThreshold = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_hitThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->hitThreshold);
	m_spinHitThreshold->SetDigits(2);

	wxStaticText* labelNoise  = new wxStaticText(this, wxID_ANY, "Noise Threshold", wxDefaultPosition, wxDefaultSize);
	m_spinNoise = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_noiseThresh, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->noiseThreshold);
	m_spinNoise->SetDigits(2);
	
	m_btnSelectRoi = new wxButton(this, CAMERA_ids::BTN_selectRoi, "Select Region of interest");
	
	m_btnSelectIgnoredAreas = new wxButton(this, CAMERA_ids::BTN_selectIgnoredAreas, "Select ignored areas");
	
	// Add widgets to sizer
	sizerLeft->Add(labelName, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(m_txtName, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxALL | wxGROW, 5);

	sizerRight->Add(labelComboType, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_comboType, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxALL | wxGROW, 5);

	sizerLeft->Add(labelUrl, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(m_txtUrl, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxALL | wxGROW, 5);
	
	sizerLeft->Add(labelOrder, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(m_spinOrder, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxALL | wxGROW, 5);

	sizerLeft->Add(labelRotation, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(m_spinRotation, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxALL | wxGROW, 5);
	
	sizerLeft->Add(labelChangeThreshold, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(m_spinChangeThreshold, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxALL | wxGROW, 5);
	
	sizerLeft->Add(m_btnSelectRoi, 0, wxALL | wxGROW, 0);
	sizerLeft->Add(5, 5, 0, wxALL | wxGROW, 5);
	
	sizerLeft->Add(m_btnSelectIgnoredAreas, 0, wxALL | wxGROW, 0);

	sizerRight->Add(labelMinimumThreshold, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinMinimumThreshold, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxALL | wxGROW, 5);

	sizerRight->Add(labelIncreaseThresholdFactor, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinIncreaseThresholdFactor, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxALL | wxGROW, 5);

	sizerRight->Add(labelUpdateThresholdFreq, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinUpdateThresholdFreq, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxALL | wxGROW, 5);

	sizerRight->Add(labelHitThreshold, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinHitThreshold, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxALL | wxGROW, 5);
	
	sizerRight->Add(labelNoise, 0, wxALL | wxGROW, 0);
	sizerRight->Add(m_spinNoise, 0, wxALL | wxGROW, 0);
	sizerRight->Add(5, 5, 0, wxALL | wxGROW, 5);
	
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
	std::cout << this->m_comboType->GetValue()  << (int)this->m_config->type << std::endl;
}

void cPanelCameraConfig::btnSelectRoi_Click(wxCommandEvent& ev) {
	AreaSelector::SelectCameraROI(this->m_config->url, this->m_config->roi);
}

void cPanelCameraConfig::btnSelectIgnoredAreas_Click(wxCommandEvent& ev) {
	AreaSelector::SelectCameraIgnoredAreas(*this->m_config);
}