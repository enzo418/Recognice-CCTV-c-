#pragma once

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/bookctrl.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/stattext.h>

#include "Recognice-CCTV-c-/src/types_configuration.hpp"
#include "Recognice-CCTV-c-/src/types.hpp"

#include "types.hpp"

enum CAMERA_ids {
	TXT_name = 101,
	TXT_url,
	SPIN_order,
	SPIN_rotation,
	SPIN_changeThreshold,
	SPIN_minThreshold,
	SPIN_increaseThreshFactor,
	SPIN_updateThresFreq,
	COMBO_type,
	SPIN_hitThreshold,
	SPIN_noiseThresh
};

class cPanelCameraConfig : public wxPanel {
	public: 
		cPanelCameraConfig(wxBookCtrlBase* parent, CameraConfiguration* camConfig, SharedData* sharedData);
		~cPanelCameraConfig();

	protected:		
		void spinOrder_Change(wxSpinEvent& ev);
		void spinRotation_Change(wxSpinEvent& ev);
		void spinChangeThreshold_Change(wxSpinEvent& ev);
		void spinMinThreshold_Change(wxSpinEvent& ev);
		void spinIncreaseThreshold_Change(wxSpinDoubleEvent& ev);
		void spinUpdateThresholdFrequency_Change(wxSpinEvent& ev);
		void txtName_KillFocus(wxFocusEvent& ev);
		void txtUrl_KillFocus(wxFocusEvent& ev);
		void spinHitThreshold_Change(wxSpinDoubleEvent& ev);
		void spinNoiseThreshold_Change(wxSpinDoubleEvent& ev);
		void comboType_Select(wxCommandEvent& ev);
private:
		SharedData* m_sharedData;
	
		CameraConfiguration* m_config;

		wxTextCtrl* m_txtName;
		wxTextCtrl* m_txtUrl;
		
		wxSpinCtrl* m_spinOrder;
		wxSpinCtrl* m_spinRotation;

		wxSpinCtrl* m_spinChangeThreshold;
		wxSpinCtrl* m_spinMinimumThreshold;
		wxSpinCtrlDouble* m_spinIncreaseThresholdFactor;
		wxSpinCtrl* m_spinUpdateThresholdFreq;

		wxComboBox* m_comboType;

		wxSpinCtrlDouble* m_spinHitThreshold;

		wxSpinCtrlDouble* m_spinNoise;

		wxDECLARE_EVENT_TABLE();
};
