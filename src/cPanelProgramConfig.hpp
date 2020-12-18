#pragma once

#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/msgdlg.h>

#include "../recognize/src/recognize.hpp"
#include "../recognize/src/types_configuration.hpp"
#include "../recognize/src/types.hpp"
#include "../recognize/src/utils.hpp"

#include "types.hpp"

enum PROGRAM_ids {
	SPIN_MsBetweenFrames = 1500,
	SPIN_RatioScaleOutput,
	CHK_ShowPreviewCameras,
	CHK_ShowAreaCameraSees,
	CHK_ShowProcessedImages,
	CHK_UseTelegramBot,
	TXT_TelegramBotApiKey,
	TXT_TelegramChatId,
	TXT_AuthUsersToSendActions,
	SPIN_SecondsBetweenImage,
	SPIN_SecondsBetweenMessage,
	CHK_SendImageAfterDetectigChange,
	CHK_SendImageOfAllCameras,
	CHK_UseGifInsteadOfImage,
	COMBO_GifQuality,
	SPIN_FramesBefore,
	SPIN_FramesAfter
};

class cPanelProgramConfig : public wxPanel {
	public: 
		cPanelProgramConfig(wxBookCtrlBase* parent, ProgramConfiguration& pConfig, SharedData* sharedData);
		~cPanelProgramConfig();

	private:
		SharedData* m_sharedData;
		
		ProgramConfiguration* m_config;

		wxSpinCtrl* m_spinMsBetweenFrames;
		wxSpinCtrlDouble* m_spinRatioScaleOutput;

		wxCheckBox *m_chkShowPreviewCameras;
		wxCheckBox *m_chkShowAreaCameraSees;
		wxCheckBox *m_chkShowProcessedImages;

		// -- Telegram Bot config
		wxCheckBox* m_chkUseTelegramBot;
		wxTextCtrl* m_txtTelegramBotApiKey;
		wxTextCtrl* m_txtTelegramChatId;
		wxTextCtrl* m_txtAuthUsersToSendActions;
		wxSpinCtrl* m_spinSecondsBetweenImage;
		wxSpinCtrl* m_spinSecondsBetweenMessage;
		wxCheckBox* m_chkSendImageAfterDetectigChange;
		wxCheckBox* m_chkSendImageOfAllCameras;
		wxCheckBox* m_chkUseGifInsteadOfImage;
		wxComboBox* m_comboGifQuality;
		wxSpinCtrl* m_spinFramesBefore;
		wxSpinCtrl* m_spinFramesAfter;

	protected:
		void spinMsBetweenFrames_SpinChange(wxSpinEvent& ev);
		void spinRatioScaleOutput_SpinChange(wxSpinDoubleEvent& ev);
		void chkShowPreviewCameras_CheckBoxClick(wxCommandEvent& ev);
		void chkShowAreaCameraSees_CheckBoxClick(wxCommandEvent& ev);
		void chkShowProcessedImages_CheckBoxClick(wxCommandEvent& ev);
		void chkUseTelegramBot_CheckBoxClick(wxCommandEvent& ev);
		void txtTelegramBotApiKey_KillFocus(wxFocusEvent& ev);
		void txtTelegramChatId_KillFocus(wxFocusEvent& ev);
		void txtAuthUsersToSendActions_KillFocus(wxFocusEvent& ev);
		void spinSecondsBetweenImage_SpinChange(wxSpinEvent& ev);
		void spinSecondsBetweenMessage_SpinChange(wxSpinEvent& ev);
		void chkSendImageAfterDetectigChange_CheckBoxClick(wxCommandEvent& ev);
		void chkSendImageOfAllCameras_CheckBoxClick(wxCommandEvent& ev);
		void chkUseGifInsteadOfImage_CheckBoxClick(wxCommandEvent& ev);
		void comboGifQuality_Select(wxCommandEvent& ev);
		void spinFramesBefore_SpinChange(wxSpinEvent& ev);
		void spinFramesAfter_SpinChange(wxSpinEvent& ev);
	private:
		wxDECLARE_EVENT_TABLE();
};