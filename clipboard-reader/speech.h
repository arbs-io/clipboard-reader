#pragma once

class CSpeech
{
private:
	std::string m_message;
	
public:
	CSpeech(std::string message);

	~CSpeech() noexcept;

	LRESULT TextToSpeech();
};

inline CSpeech::CSpeech(std::string message): m_message(std::move(message))
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		throw std::runtime_error("CoInitialize Failed");
	}
}

inline CSpeech::~CSpeech() noexcept
{
	CoUninitialize();
}

inline LRESULT CSpeech::TextToSpeech()
{	
	ISpVoice * pVoice = nullptr;
		
	auto hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	if (SUCCEEDED(hr))
	{
		auto nws = std::wstring(m_message.begin(), m_message.end());
		pVoice->Speak(nws.c_str(), 0, nullptr);			
		pVoice->Release();
		pVoice = nullptr;
	}

	return TRUE;
}
