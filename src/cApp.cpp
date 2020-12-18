#include "cApp.hpp"

wxIMPLEMENT_APP(cApp);

cApp::cApp() {
}

cApp::~cApp() {

}

bool cApp::OnInit(){
	this->m_appConfig = new wxConfig("wxRecognize");
	
	this->m_startRecognizeOnStart = this->m_appConfig->Read("StartRecognizeOnStart", false);
	
	wxString lastConfigFile = this->m_appConfig->Read("LastConfigFile", "config.ini");

	// Recognize will live as long as the app is live
	this->m_recognize = new Recognize();

	// Get the cameras and program configurations
	this->configurations = ConfigurationFile::ReadConfigurations(lastConfigFile.ToStdString());

	if (this->m_startRecognizeOnStart)
		this->m_recognize->Start(std::ref(this->configurations), false, this->configurations.programConfig.telegramConfig.useTelegramBot);
	
	// Main frame, also loads the file with the config and starts recognize
	this->m_main = new cMain(this->m_recognize, this->configurations, this->m_startRecognizeOnStart, this->m_appConfig, this->m_mainClosed, lastConfigFile.ToStdString());
	this->m_main->Show();

	// Window (thread) that never is shown. Leaves all the window (thread) to a cv::imshow window
	this->m_preview = new cPreviewCameras(*m_recognize, this->configurations, this->m_startRecognizeOnStart, this->m_mainClosed);

	delete this->m_appConfig;

	return true;
}

void cApp::TogglePreview(Configurations& configs) {

}