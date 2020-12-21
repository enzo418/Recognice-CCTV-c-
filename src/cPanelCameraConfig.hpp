#pragma once

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/bookctrl.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/stattext.h>

#include "../recognize/src/types_configuration.hpp"
#include "../recognize/src/types.hpp"

#include "AreaSelector.hpp"

#include "types.hpp"

#include "widgets_helper.hpp"

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
	SPIN_noiseThresh,
	BTN_selectRoi,
	BTN_selectIgnoredAreas,
	SPIN_FramesAnalyzeBefore,
	SPIN_FramesAnalyzeAfter
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
		void btnSelectRoi_Click(wxCommandEvent& ev);
		void btnSelectIgnoredAreas_Click(wxCommandEvent& ev);
		
		void spinFramesAnalyzeBefore_Change(wxSpinEvent& ev);
		void spinFramesAnalyzeAfter_Change(wxSpinEvent& ev);
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
		
		wxButton* m_btnSelectRoi;
		wxButton* m_btnSelectIgnoredAreas;
		
		wxSpinCtrl* m_spinFramesAnalyzeBefore;
		wxSpinCtrl* m_spinFramesAnalyzeAfter;
};
