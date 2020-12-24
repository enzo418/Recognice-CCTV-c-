#include "cPanelCameraConfig.hpp"

#define BIND(Widget_, EventTag_, EventHandler_, Function_) Widget_->Bind(EventTag_, \
																			[this](EventHandler_& ev) { \
																				this->m_sharedData->btnApplyChanges->Enable(true); \
																				Function_(ev); \
																			})

cPanelCameraConfig::cPanelCameraConfig(wxBookCtrlBase* parent, CameraConfiguration* camConfig, SharedData* sharedData) 
		: 	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN),
			m_config(camConfig), m_sharedData(sharedData) {
		
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

	m_spinChangeThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_changeThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->changeThreshold);
	BIND(m_spinChangeThreshold, wxEVT_SPINCTRL, wxSpinEvent, spinChangeThreshold_Change);
	
	m_spinMinimumThreshold = new wxSpinCtrl(this, CAMERA_ids::SPIN_minThreshold, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->minimumThreshold);
	BIND(m_spinMinimumThreshold, wxEVT_SPINCTRL, wxSpinEvent, spinMinThreshold_Change);
	
	m_spinIncreaseThresholdFactor = new wxSpinCtrlDouble(this, CAMERA_ids::SPIN_increaseThreshFactor, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, dmax, m_config->increaseTresholdFactor);
	m_spinIncreaseThresholdFactor->SetDigits(2);
	BIND(m_spinIncreaseThresholdFactor, wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent, spinIncreaseThreshold_Change);
	
	m_spinUpdateThresholdFreq = new wxSpinCtrl(this, CAMERA_ids::SPIN_updateThresFreq, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, imax, m_config->updateThresholdFrequency);
	BIND(m_spinUpdateThresholdFreq, wxEVT_SPINCTRL, wxSpinEvent, spinUpdateThresholdFrequency_Change);
	
	m_comboType = new wxComboBox(this, CAMERA_ids::COMBO_type, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	m_comboType->Append(wxString(_("Active")));
	m_comboType->Append(wxString(_("Disabled")));
	m_comboType->Append(wxString(_("Sentry")));
	m_comboType->SetValue((this->m_config->type == CAMERA_DISABLED ? _("Disabled") : (this->m_config->type == CAMERA_SENTRY ? _("Sentry") : _("Active"))));
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
	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtName, _("Name")), 0, flags, border);

	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_txtUrl, _("Url")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_comboType, _("Camera type")), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinOrder, _("Order")),
						WidgetsHelper::GetSizerItemLabel(this, m_spinRotation, _("Rotation")),
						5
					), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinChangeThreshold, _("Change Threshold")), 0, flags, border);
	
	sizerLeft->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						m_btnSelectRoi,
						m_btnSelectIgnoredAreas,
						5
					), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinMinimumThreshold, _("Minimum Threshold")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinIncreaseThresholdFactor, _("Increase threshold factor")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::GetSizerItemLabel(this, m_spinUpdateThresholdFreq, _("Update threshold frequency")), 0, flags, border);

	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinHitThreshold, _("Hit threshold")),
						WidgetsHelper::GetSizerItemLabel(this, m_spinNoise, _("Noise threshold")),
						5
					), 0, flags, border);
	
	sizerRight->Add(WidgetsHelper::JoinWidgetsOnSizerH(
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAnalyzeBefore, _("Frames Analyze Before")),
						WidgetsHelper::GetSizerItemLabel(this, m_spinFramesAnalyzeAfter, _("Frames Analyze After")),
						5
					), 0, flags, border);
	
	sizerRight->Add(m_spinMinPercentageAreaIgnored, 0, flags, border);
		
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
	this->m_config->type = (t == _("Disabled") ? 0 : (t == _("Sentry") ? 1 : 2));
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