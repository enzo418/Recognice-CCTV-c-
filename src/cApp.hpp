#pragma once
#include "cMain.hpp"
#include "cPreviewCameras.hpp"

#include <wx/app.h>
#include <wx/event.h>
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#ifdef _WIN32 
#include <wx/msw/regconf.h>
#endif

class cApp : public wxApp {
	public:
		cApp();
		~cApp();

		void TogglePreview(Configurations& configs);
		virtual bool OnInit();
		void OnQuitMain(wxCloseEvent& event);
	private:
		wxConfig* m_appConfig;
		cMain* m_main = nullptr;
		
		cPreviewCameras* m_preview = nullptr;
		Configurations configurations;
		Recognize* m_recognize = nullptr;

		bool m_startRecognizeOnStart;
		bool m_mainClosed = false;
};